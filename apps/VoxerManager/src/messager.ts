import WebSocket from "ws";
import mongodb from "mongodb";
import { resolve } from "path";
import { WORKER, UPLOAD_PATH } from "./config";
import Dataset from "./models/Dataset";

class Messager {
  ws: WebSocket;
  pingTimeout: NodeJS.Timer;
  pingInterval: number;
  reconnectInterval: number;
  cache: Record<
    string,
    {
      dimensions: [number, number, number];
      histogram: Array<number>;
    }
  >;

  constructor() {
    this.pingInterval = 5 * 1000;
    this.reconnectInterval = 5 * 1000;
    this.cache = {};
  }

  post = (data: unknown) => {
    if (!this.ws) {
      return;
    }

    try {
      this.ws.send(JSON.stringify(data));
    } catch (e) {
      this.ws.emit("error", e);
    }
  };

  connect(database: mongodb.Db) {
    this.close();

    this.ws = new WebSocket(`${WORKER}/datasets`);
    this.ws.on("open", async () => {
      console.log("DatasetMessager: connected");

      const collection = database.collection("datasets");
      const datasets = await collection.find().toArray();

      this.post(
        datasets.map((dataset: Dataset) => ({
          id: dataset._id.toHexString(),
          name: dataset.name,
          path: resolve(UPLOAD_PATH, dataset.path),
        }))
      );

      this.pingTimeout = setInterval(() => {
        this.ws.ping();
      }, this.pingInterval);
    });
    this.ws.on("close", (code, reason) => {
      switch (code) {
        case 1000: // CLOSE_NORMAL
          console.log("DatasetMessager: closed", reason);
          break;
        default:
          this.reconnect(database);
          break;
      }
    });
    this.ws.onmessage = (msg) => {
      const { data } = msg;
      try {
        const json = JSON.parse(data as string);

        this.cache[json.id] = json;
      } catch (e) {
        console.log("不支持的消息: ", data);
      }
      return;
    };
    this.ws.onerror = (err) => {
      console.log("DatasetMessager Error: ", err.message);
      switch (err.type) {
        case "ECONNREFUSED": {
          clearInterval(this.pingTimeout);
          this.reconnect(database);
          break;
        }
        default: {
          break;
        }
      }
    };
  }

  close = () => {
    if (!this.ws) {
      return;
    }

    this.ws.close();
    clearInterval(this.pingTimeout);
  };

  reconnect = (database: mongodb.Db) => {
    this.ws.removeAllListeners();
    setTimeout(() => {
      console.log("DatasetMessager: reconnecting...");
      this.connect(database);
    }, this.reconnectInterval);
  };
}

export default new Messager();
