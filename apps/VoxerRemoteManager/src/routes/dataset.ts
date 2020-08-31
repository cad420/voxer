import express from "express";
import multer from "multer";
import Dataset from "../models/Dataset";
import { PUBLIC_PATH, UPLOAD_PATH } from "../config";
import messager from "../messager";
import { resolve } from "path";

const router = express.Router();

const storage = multer.diskStorage({
  destination: (req, file, cb) => {
    cb(null, PUBLIC_PATH);
  },
  filename: (req, file, cb) => {
    cb(null, file.originalname);
  },
});
const upload = multer({ storage });

router.post("/", upload.single("dataset"), async (req, res) => {
  const { name, variable = "default", timestep = "0" } = req.params;
  const path = req.file.fieldname;

  // TODO: make sure previous timestep is uploaded

  const dataset = await Dataset.findOne({
    where: {
      name,
      variable,
      timestep: parseInt(timestep),
    },
  });

  if (!dataset) {
    await Dataset.create({
      name,
      variable,
      timestep: parseInt(timestep),
      path,
    });
  } else {
    dataset.path = path;
    await dataset.save();
  }

  messager.post({
    id: dataset.id,
    name: dataset.name,
    variable: dataset.variable,
    timestep: dataset.timestep,
    path: resolve(UPLOAD_PATH, dataset.path.substr(1)),
  });

  res.send({
    code: 200,
    data: name,
  });
});

router.get("/", async (req, res) => {
  const datasets = await Dataset.findAll();

  res.send({
    code: 200,
    data: datasets.map(({ id, name, variable, timestep }) => ({
      id,
      name,
      variable,
      timestep,
      dimensions: messager.cache[id.toString()].dimensions
    })),
  });
});

router.get("/collections", async (req, res) => {
  const datasets = await Dataset.findAll();

  const collections: Record<
    string,
    {
      name: string;
      variables: Array<{
        name: string;
        timesteps: number;
      }>;
    }
  > = {};

  datasets.forEach((dataset) => {
    if (!collections[dataset.name]) {
      collections[dataset.name] = {
        name: dataset.name,
        variables: [],
      };
    }

    const collection = collections[dataset.name];
    const variable = collection.variables.find(
      (item) => item.name === dataset.variable
    );
    if (variable) {
      variable.timesteps = Math.max(variable.timesteps, dataset.timestep + 1);
    } else {
      collection.variables.push({
        name: dataset.variable,
        timesteps: dataset.timestep + 1,
      });
    }
  });

  res.send({
    code: 200,
    data: Object.values(collections),
  });
});

router.get("/:name/:variable/:timestep", async (req, res) => {
  const { name, variable, timestep } = req.params;

  const dataset = await Dataset.findOne({
    where: {
      name,
      variable,
      timestep: parseInt(timestep),
    },
  });

  if (!dataset) {
    res.send({
      code: 404,
      data: "Not found",
    });
    return;
  }

  // TODO: send histogram
  return res.send({
    code: 200,
    data: [],
  });
});

export default router;
