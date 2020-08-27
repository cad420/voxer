import WebSocket from "ws";
import Dataset from "./models/Dataset";
import { RENDER_SERVICE, UPLOAD_PATH } from "./config";
import {resolve } from 'path'

class Messager {
  ws: WebSocket;
  pingTimeout: NodeJS.Timer;
  pingInterval: number;
  reconnectInterval: number;

  constructor() {
    this.pingInterval = 5 * 1000;
    this.reconnectInterval = 5 * 1000;
    this.connect();
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

  connect() {
    this.close();

    this.ws = new WebSocket(`${RENDER_SERVICE}/datasets`);
    this.ws.on("open", async () => {
      console.log("DatasetMessager: connected");

      const datasets = await Dataset.findAll();

      this.post(
        datasets.map(({ id, name, variable, timestep, path }) => ({
          id,
          name,
          variable,
          timestep,
          path: resolve(UPLOAD_PATH, path.substr(1)),
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
          this.reconnect();
          break;
      }
    });
    this.ws.onerror = (err) => {
      console.log("DatasetMessager Error: ", err.message);
      switch (err.type) {
        case "ECONNREFUSED": {
          clearInterval(this.pingTimeout);
          this.reconnect();
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

  reconnect = () => {
    this.ws.removeAllListeners();
    setTimeout(() => {
      console.log("DatasetMessager: reconnecting...");
      this.connect();
    }, this.reconnectInterval);
  };
}

export default new Messager();
