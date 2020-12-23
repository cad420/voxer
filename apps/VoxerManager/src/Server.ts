import express from "express";
import WebSocket from "ws";
import http from "http";
import net from "net";
import { PUBLIC_PATH, DATABASE } from "./config";
import cors from "cors";
import routes from "./routes";
import { Db, MongoClient } from "mongodb";
import pino from "pino";
import nanoid from "nanoid";

const msgpack = require("@ygoe/msgpack");

const logger = pino();

class Server {
  workers: WebSocket[];
  clients: Map<string, WebSocket>;
  workerIdx: number;
  publicDir: string;
  database: Db;
  port: number;

  constructor() {
    this.workers = [];
    this.clients = new Map();
    this.publicDir = PUBLIC_PATH;
    this.port = 3001;
    this.workerIdx = 0;
  }

  async connectDatabase() {
    const client = new MongoClient(`mongodb://${DATABASE}?w=majority`, {
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
    logger.info("new client");

    const id = nanoid(10);

    this.clients.set(id, ws);

    const info = msgpack.serialize({ method: "connect", id });
    ws.send(info);

    ws.on("message", (message) => {
      const workerNum = this.workers.length;

      if (workerNum <= 0) {
        const error = "no available worker";
        logger.warn(error);
        ws.send(
          msgpack.serialize({ id, error: { code: 500, message: error } })
        );
        return;
      }

      const worker = this.workers[this.workerIdx];
      this.workerIdx = (this.workerIdx + 1) % workerNum;
      worker.send(message);
    });

    ws.on("close", () => {
      logger.warn("Client Close");
      this.clients.delete(id);
    });

    ws.on("error", (msg: WebSocket.Data) => {
      logger.warn(`Client Error: ${msg.toString()}`);
    });
  };

  handleWorkerConnect = (ws: WebSocket) => {
    logger.info("new worker");

    const index = this.workers.length;
    this.workers.push(ws);

    ws.on("message", this.handleWorkerMessage);

    ws.on("close", () => {
      logger.warn("Worker Close");
      this.workers = this.workers.splice(index, 1);
      if (this.workerIdx > index) {
        this.workerIdx = this.workerIdx % this.workers.length;
      }
    });

    ws.on("error", (msg: WebSocket.Data) => {
      logger.warn(`Worker Error: ${msg.toString()}`);
    });
  };

  handleWorkerMessage = (message: WebSocket.Data) => {
    let id = "";
    if (message instanceof Buffer) {
      const idLen = Math.min(message.readUInt8(4) ^ 0x10100000, 10);
      id = message.toString("utf8", 5, 5 + idLen);
    }

    const client = this.clients.get(id);
    if (!client) {
      return;
    }

    client.send(message);
  };

  async listen() {
    await this.connectDatabase();

    const app = express();

    app.use(express.static(PUBLIC_PATH));
    app.use(cors());
    app.use(express.json());
    app.use(routes);

    app.use((req, res) => {
      res.status(404);
      res.end("Not found.");
    });

    app.set("database", this.database);

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

    return new Promise((resolve) => {
      server.listen(this.port, () => {
        logger.error(`Listening on port ${this.port}`);
        resolve();
      });
    });
  }
}

export default Server;
