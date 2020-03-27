import express from "express";
import multer from "multer";
import path from "path";
import store from "../models/Dataset";
import { PUBLIC_PATH } from "../config";

const router = express.Router();

const storage = multer.diskStorage({
  destination: (req, file, cb) => {
    cb(null, PUBLIC_PATH);
  },
  filename: (req, file, cb) => {
    cb(null, file.originalname);
  }
});
const upload = multer({ storage });

router.post("/", upload.single("dataset"), (req, res) => {
  const name = req.file.filename;
  const dataset = {
    name,
    variables: [
      {
        name: "default",
        timesteps: 1,
        path: path.resolve(PUBLIC_PATH, name)
      }
    ]
  };
  store.add(dataset);

  res.send({
    code: 200,
    data: name
  });
});

router.get("/", (req, res) => {
  const datasets = store.getAll();

  res.send({
    code: 200,
    data: Object.values(datasets).map(({ name }) => ({ name }))
  });
});

router.get("/:name/:variable/:timestep", (req, res) => {
  const { name, variable, timestep } = req.params;
  const timestepValue = parseInt(timestep);

  const dataset = store.get(name);
  if (!dataset) {
    res.send({
      code: 404,
      data: "Not found"
    });
    return;
  }

  const variables = dataset.variables;
  const datasetVariable = variables.find(item => item.name === variable);
  if (!datasetVariable) {
    res.send({
      code: 404,
      data: "Not found"
    });
    return;
  }

  if (datasetVariable.timesteps < timestepValue) {
    res.send({
      code: 404,
      data: "Not found"
    });
    return;
  }

  res.send({
    code: 200,
    data: store.getHistogram(name, variable, timestepValue)
  });
});

export default router;
