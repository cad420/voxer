import WebSocket from "ws";
import { Dataset } from "./voxer";
import { RENDER_SERVICE } from "../config";

class DatasetMessager {
  ws: WebSocket;
  pingTimeout: NodeJS.Timer;
  pingInterval: number;
  reconnectInterval: number;
  saved: Dataset[];

  constructor() {
    this.pingInterval = 5 * 1000;
    this.reconnectInterval = 5 * 1000;
    this.saved = [];
    this.connect();
  }

  post = (data: Dataset | Dataset[]) => {
    if (Array.isArray(data)) {
      this.saved = this.saved.concat(data);
    } else {
      this.saved.push(data);
    }

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
    this.ws.on("open", () => {
      console.log("DatasetMessager: connected");
      this.post(this.saved);
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
    this.ws.onerror = err => {
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

export default DatasetMessager;
