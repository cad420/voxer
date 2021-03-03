import express from "express";
import WebSocket from "ws";
import http from "http";
import net from "net";
import cors from "cors";
import routes from "./routes";
import { Db, MongoClient } from "mongodb";
import pino from "pino";
import nanoid from "nanoid";
import WorkerAPICaller from "./worker_api";

const msgpack = require("@ygoe/msgpack");

const logger = pino();

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

  constructor(options: ServerOptions) {
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

  async connectDatabase() {
    const client = new MongoClient(`mongodb://${this.databseAddr}?w=majority`, {
      useUnifiedTopology: true,
      useNewUrlParser: true,
    });

    try {
      await client.connect();

      // Establish and verify connection
      const db = await client.db("voxer");
      await db.command({ ping: 1 });

      logger.info("Connected successfully to database");

      this.database = db;
    } catch (e) {
      logger.info("Failed to connect to server: ", e.message);
      await client.close();
    }
  }

  handleClientConnect = (ws: WebSocket) => {
    const id = nanoid(10);

    const client = ws as any;
    client.isAlive = true;
    ws.on("pong", () => {
      client.isAlive = true;
    });
    this.clients.set(id, ws);

    logger.info(`New client connected, id: ${id}`);

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
        logger.warn(error);
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
      logger.warn(`Client(${id}) Closed`);
      this.clients.delete(id);
    });

    ws.on("error", (msg: WebSocket.Data) => {
      logger.warn(`Client(${id}) Error: ${msg.toString()}`);
    });
  };

  handleWorkerConnect = (ws: WebSocket) => {
    logger.info("New worker connected");

    const worker = ws as any;
    worker.isAlive = true;
    ws.on("pong", () => {
      worker.isAlive = true;
    });

    const index = this.workers.length;
    this.workers.push(ws);

    ws.on("message", this.handleWorkerMessage);

    ws.on("close", () => {
      logger.warn("Worker Closed");
      this.workers.splice(index, 1);
      if (this.workerIdx > index) {
        this.workerIdx = this.workerIdx % this.workers.length;
      }
    });

    ws.on("error", (msg: WebSocket.Data) => {
      logger.warn(`Worker Error: ${msg.toString()}`);
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
    await this.connectDatabase();

    const app = express();

    app.use(express.static(this.publicDir));
    app.use(cors());
    app.use(express.json());
    app.use(routes);

    app.use((req, res) => {
      res.status(404);
      res.end("Not found.");
    });

    app.set("database", this.database);
    app.set("worker", this.worker);

    const server = http.createServer(app);

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

    return new Promise<void>((resolve) => {
      server.listen(this.port, () => {
        logger.error(`Listening on port ${this.port}`);
        resolve();
      });
    });
  }
}

export default Server;
