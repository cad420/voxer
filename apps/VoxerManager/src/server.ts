import express from "express";
import cors from "cors";
import http from "http";
import net from "net";
import WebSocket from "ws";
import routes from "./routes";
import { PUBLIC_PATH } from "./config";
import connect from "./models";

async function run(port: number) {
  const app = express();

  app.use(express.static(PUBLIC_PATH));
  app.use(cors());
  app.use(express.json());
  app.use(routes);

  app.use((req, res) => {
    res.status(404);
    res.end("Not found.");
  });

  try {
    const database = await connect();

    app.set("database", database);

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

    let workers: WebSocket[] = [];
    let clients: WebSocket[] = [];
    wss.on("connection", (ws, req) => {
      let isClient = false;
      let index = 0;
      if (req.url === "/worker") {
        workers.push(ws);
        index = workers.length;
      } else if (req.url === "/rpc") {
        workers.push(ws);
        index = clients.length;
        isClient = true;
      } else {
        ws.close();
        return;
      }

      if (isClient) {
        ws.on("message", (message) => {
          // TODO: select worker, send id and relay message
          console.log(message);
        });

        ws.on("close", () => {
          console.log("closed");
          clients = clients.splice(index, 1);
        });

        ws.on("error", () => {
          console.log(`${index} error`);
          clients = clients.splice(index, 1);
        });
      } else {
        ws.on("message", (message) => {
          // TODO: extract client id, data & relay data
          console.log(message);
        });

        ws.on("close", () => {
          console.log("closed");
          workers = workers.splice(index, 1);
        });

        ws.on("error", () => {
          console.log(`${index} error`);
          workers = workers.splice(index, 1);
        });
      }
    });

    server.listen(port, function () {
      console.log(`Listening on port ${port}`);
    });
  } catch (err) {
    console.error(err.message);
  }
}

export default run;
