import http from "http";
import net from "net";
import path from "path";
import pino from "pino";
import WebSocket from "ws";
import nanoid from "nanoid";
import { Db } from "mongodb";
import FastifyJWTPlugin from "fastify-jwt";
import FastifyCORSPlugin from "fastify-cors";
import FastifyStaticPlugin from "fastify-static";
import FastifyHelmetPlugin from "fastify-helmet";
import FastifySwaggerPlugin from "fastify-swagger";
import FastifyMongoDBPlugin from "fastify-mongodb";
import FastifyMultiPartPlugin from "fastify-multipart";
import fastify, { FastifyLoggerInstance, FastifyServerFactory } from "fastify";
import routes from "./routes";
import openapi from "./openapi";
import { isProduction } from "./index";
import WorkerAPICaller from "./worker_api";

declare module "fastify" {
  interface FastifyInstance {
    getWorker: () => WorkerAPICaller;
  }
}

const msgpack = require("@ygoe/msgpack");

interface ServerOptions {
  port?: number;
  serve?: string;
  database?: string;
}

class Server {
  workers: WebSocket[];
  clients: Map<string, WebSocket>;
  workerIdx: number;
  publicDir: string;
  database: Db;
  databseAddr: string;
  port: number;
  worker: WorkerAPICaller;
  logger: FastifyLoggerInstance;

  constructor(options: ServerOptions) {
    this.logger = pino();
    this.workers = [];
    this.clients = new Map();
    this.publicDir = options.serve || "public";
    this.port = options.port || 3001;
    this.databseAddr = options.database || "127.0.0.1:27017";
    this.workerIdx = 0;
    this.worker = new WorkerAPICaller();
    this.worker.send = (message) => {
      const workerNum = this.workers.length;

      if (workerNum <= 0) {
        throw new Error("no available worker");
      }

      const worker = this.workers[this.workerIdx];
      this.workerIdx = (this.workerIdx + 1) % workerNum;
      const data = msgpack.serialize(message);
      worker.send(data);
    };
  }

  handleClientConnect = (ws: WebSocket) => {
    const id = nanoid(10);

    const client = ws as any;
    client.isAlive = true;
    ws.on("pong", () => {
      client.isAlive = true;
    });
    this.clients.set(id, ws);

    this.logger.info(`New client connected, id: ${id}`);

    const info = msgpack.serialize({
      caller: id,
      method: "connect",
      id,
      result: id,
    });
    ws.send(info);

    ws.on("message", (message) => {
      const workerNum = this.workers.length;

      if (workerNum <= 0) {
        const error = "no available worker";
        this.logger.warn(error);
        const data = msgpack.serialize({
          caller: id,
          id,
          error: { code: 500, message: error },
        });
        ws.send(data);
        return;
      }

      const worker = this.workers[this.workerIdx];
      this.workerIdx = (this.workerIdx + 1) % workerNum;
      worker.send(message);
    });

    ws.on("close", () => {
      this.logger.warn(`Client(${id}) Closed`);
      this.clients.delete(id);
    });

    ws.on("error", (msg: WebSocket.Data) => {
      this.logger.warn(`Client(${id}) Error: ${msg.toString()}`);
    });
  };

  handleWorkerConnect = (ws: WebSocket) => {
    this.logger.info("New worker connected");

    const worker = ws as any;
    worker.isAlive = true;
    ws.on("pong", () => {
      worker.isAlive = true;
    });

    const index = this.workers.length;
    this.workers.push(ws);

    ws.on("message", this.handleWorkerMessage);

    ws.on("close", () => {
      this.logger.warn("Worker Closed");
      this.workers.splice(index, 1);
      if (this.workerIdx > index) {
        this.workerIdx = this.workerIdx % this.workers.length;
      }
    });

    ws.on("error", (msg: WebSocket.Data) => {
      this.logger.warn(`Worker Error: ${msg.toString()}`);
    });
  };

  handleWorkerMessage = (message: WebSocket.Data) => {
    if (!(message instanceof Buffer)) {
      return;
    }

    // see https://github.com/msgpack/msgpack/blob/master/spec.md#str-format-family
    const callerLenByteOffset = 8; // { "caller": "xxxxxx", ... }
    const callerByteOffset = callerLenByteOffset + 1;
    const callerLen = message.readUInt8(callerLenByteOffset) ^ 0b10100000;
    const caller = message.toString(
      "utf8",
      callerByteOffset,
      callerByteOffset + callerLen
    );

    if (caller === "VoxerManager") {
      // RPC called by manager
      this.worker.responsed(message);
      return;
    }

    const client = this.clients.get(caller);
    if (!client) {
      return;
    }

    client.send(message);
  };

  checkAlive = () => {
    setInterval(() => {
      this.workers.forEach((worker) => {
        const ws = worker as any;
        if (ws.isAlive === false) {
          return ws.close();
        }

        ws.isAlive = false;
        ws.ping();
      });
    }, 5 * 1000);

    setInterval(() => {
      this.clients.forEach((client) => {
        const ws = client as any;
        if (ws.isAlive === false) {
          return ws.close();
        }

        ws.isAlive = false;
        ws.ping();
      });
    }, 10 * 1000);
  };

  async listen() {
    const serverFactory: FastifyServerFactory = (handler) => {
      const server = http.createServer((req, res) => {
        handler(req, res);
      });

      const wss = new WebSocket.Server({
        clientTracking: false,
        noServer: true,
      });

      server.on(
        "upgrade",
        (request: http.IncomingMessage, socket: net.Socket, head: Buffer) => {
          wss.handleUpgrade(request, socket, head, (ws) => {
            wss.emit("connection", ws, request);
          });
        }
      );

      wss.on("connection", (ws, req) => {
        if (req.url === "/worker") {
          this.handleWorkerConnect(ws);
        } else if (req.url === "/rpc") {
          this.handleClientConnect(ws);
        } else {
          ws.close();
          return;
        }
      });

      return server;
    };

    const app = fastify({ serverFactory, logger: true });

    if (isProduction) {
      app.register(FastifyHelmetPlugin);
    } else {
      app.register(FastifySwaggerPlugin, {
        routePrefix: "/doc",
        openapi,
        exposeRoute: true,
        uiConfig: {
          persistAuthorization: true,
        },
      } as any);
      app.ready((err) => {
        if (err) throw err;
        app.swagger();
      });
    }
    app.register(FastifyStaticPlugin, {
      root: path.resolve(process.cwd(), this.publicDir),
      prefix: "/",
    });
    app.register(FastifyMongoDBPlugin, {
      forceClose: true,
      url: `mongodb://${this.databseAddr}/voxer`,
    });
    app.register(FastifyCORSPlugin, {});
    app.register(FastifyMultiPartPlugin);
    app.register(FastifyJWTPlugin, {
      secret: "shhhhhhared-secret",
    });
    app.register(routes);
    app.decorate("getWorker", () => this.worker);
    this.logger = app.log;

    try {
      const address = await app.listen(this.port, "0.0.0.0");
      app.log.info(`Listening on ${address}`);
    } catch (err) {
      app.log.error(err);
    }
  }
}

export default Server;
