import express from "express";
import cors from "cors";
import { createProxyMiddleware } from "http-proxy-middleware";
import routes from "./routes";
import { PUBLIC_PATH, RENDER_SERVICE } from "./config";

const app = express();
const wsProxy = createProxyMiddleware("/render", {
  target: RENDER_SERVICE,
  ws: true
});

app.use(wsProxy);
app.use(express.static(PUBLIC_PATH));
app.use(cors());
app.use(express.json());
app.use(routes);

app.use((req, res) => {
  res.status(404);
  res.end("Not found.");
});

const port = process.env.PORT || 3001;
app.listen(port, () => {
  console.log(`listening on port ${port}`);
});
