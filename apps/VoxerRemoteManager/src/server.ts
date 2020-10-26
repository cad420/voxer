import express from "express";
import cors from "cors";
import { createProxyMiddleware } from "http-proxy-middleware";
import routes from "./routes";
import { PUBLIC_PATH, RENDER_SERVICE } from "./config";
import connect from "./models";
import { getExistDatasetInfo } from "./models/Dataset";

const app = express();

const services: string[] = ["/render", "/slice"];
services.forEach((service) => {
  const serviceProxy = createProxyMiddleware(service, {
    target: `ws://${RENDER_SERVICE}`,
    ws: true,
  });
  app.use(serviceProxy);
});

app.use(express.static(PUBLIC_PATH));
app.use(cors());
app.use(express.json());
app.use(routes);

app.use((req, res) => {
  res.status(404);
  res.end("Not found.");
});

const port = process.env.PORT || 3001;

export default async function run() {
  const database = await connect();
  const datasets: Record<string, any> = {};

  getExistDatasetInfo(database, datasets);

  app.set("database", database);
  app.set("datasets", datasets);
  app.listen(port, () => {
    console.log(`listening on port ${port}`);
  });
}

run();
