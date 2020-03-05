import express from "express";
import cors from "cors";
import routes from "./routes";
import "./models/Dataset";

const app = express();

app.use(express.static("public"));
app.use(cors());
app.use(routes);

app.use((req, res, next) => {
  res.status(404);
  res.end("Not found.");
});

const port = process.env.PORT || 3001;
app.listen(port, () => {
  console.log(`listening on port ${port}`);
});
