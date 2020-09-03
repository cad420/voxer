import express from "express";
import multer from "multer";
import mongodb from "mongodb";
import { resolve } from "path";
import Dataset from "../models/Dataset";
import { PUBLIC_PATH, UPLOAD_PATH } from "../config";
import messager from "../messager";
import DatasetGroup from "../models/DatasetGroup";

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
  const filename = req.file.filename;

  // TODO: make sure previous timestep is uploaded
  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("datasets");
  const result = await collection.insertOne({
    path: filename, // TODO
    name: filename,
    annotations: { z: [], y: [], x: [] },
  });
  const id = result.insertedId;

  messager.post({
    id,
    name: filename,
    path: resolve(UPLOAD_PATH, filename),
  });

  res.send({
    code: 200,
    data: id,
  });
});

router.get("/", async (req, res) => {
  const database: mongodb.Db = req.app.get("database");

  const collection = database.collection("datasets");
  const datasets = await collection.find({}).toArray();

  res.send({
    code: 200,
    data: datasets.map((dataset: Dataset) => ({
      id: dataset._id,
      dimensions: messager.cache[dataset._id.toString()].dimensions,
    })),
  });
});

router.get("/groups", async (req, res) => {
  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("dataset_groups");

  const groups: DatasetGroup[] = await collection.find({}).toArray();

  res.send({
    code: 200,
    data: groups.map((group) => {
      return {
        name: group.name,
        variables: group.variables.map((variable) => {
          return {
            name: variable.name,
            timesteps: variable.timesteps.map((id) => {
              return {
                id,
                dimensions: messager.cache[id.toHexString()],
              };
            }),
          };
        }),
      };
    }),
  });
});

router.post("/groups", async (req, res) => {
  const { id, name, variable = "default" } = req.params;
  const timestep = parseInt(req.params.timestep || "0");

  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("dataset_groups");
  const group: DatasetGroup = await collection.findOne({ name });

  // name not exit
  if (!group) {
    await collection.insertOne({
      name,
      variables: [
        {
          name: variable,
          timesteps: [id],
        },
      ],
    });
    res.send({ code: 200 });
    return;
  }

  // variable not exist
  const variableData = group.variables.find((item) => item.name === variable);
  if (!variableData) {
    await collection.updateOne(
      {
        _id: group._id,
      },
      {
        $set: {
          variables: [
            {
              name: variable,
              timesteps: [group],
            },
          ],
        },
      }
    );

    return;
  }

  // check timestep
  if (variableData.timesteps.length !== timestep) {
    res.send({
      code: 404,
      data: "missing timestep",
    });
    return;
  }

  await collection.updateOne(
    {
      _id: group._id,
      "variables.name": variable,
    },
    {
      $push: {
        timesteps: id,
      },
    }
  );
});

router.get("/:id", async (req, res) => {
  const { id } = req.params;
  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("datasets");

  const dataset = await collection.findOne({ _id: id });

  if (!dataset) {
    res.send({
      code: 404,
      data: "Not found",
    });
    return;
  }

  // TODO: send histogram & dimensiosn
  return res.send({
    code: 200,
    data: messager.cache[id],
  });
});

export default router;
