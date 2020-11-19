import express from "express";
import multer from "multer";
import mongodb, { ObjectID } from "mongodb";
import { resolve } from "path";
import Dataset from "../models/Dataset";
import { RENDER_SERVICE, UPLOAD_PATH } from "../config";
import { getDatasetInfo } from "../rpc";

const router = express.Router();

const storage = multer.diskStorage({
  destination: (req, file, cb) => {
    cb(null, UPLOAD_PATH);
  },
  filename: (req, file, cb) => {
    cb(null, file.originalname);
  },
});
const upload = multer({ storage });

router.get("/", async (req, res) => {
  const database: mongodb.Db = req.app.get("database");

  const collection = database.collection("datasets");
  const datasets = await collection.find().toArray();

  res.send({
    code: 200,
    data: datasets.map((dataset: Dataset) => {
      return {
        id: dataset._id,
        name: dataset.name,
        dimensions: dataset.dimensions || [1, 1, 1],
      };
    }),
  });
});

router.post("/", upload.single("dataset"), async (req, res) => {
  const filename = req.file.filename;

  // TODO: make sure previous timestep is uploaded
  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("datasets");
  const result = await collection.insertOne({
    path: filename, // TODO
    name: filename,
  });
  const id = result.insertedId;

  const info = await getDatasetInfo(RENDER_SERVICE, {
    id,
    name: filename,
    path: resolve(UPLOAD_PATH, filename),
  });

  if (info.error) {
    console.log(`Error: ${info.error}`);
    res.send({
      code: 400,
      data: info.error,
    });
    return;
  }

  collection.updateOne(
    { id: new ObjectID(id) },
    {
      $set: {
        dimensions: info.dimensions,
        histogram: info.histogram,
        range: info.range,
      },
    }
  );

  res.send({
    code: 200,
    data: id,
  });
});

router.get("/:id", async (req, res) => {
  const { id } = req.params;

  const database: mongodb.Db = req.app.get("database");

  const collection = database.collection("datasets");

  const dataset: Dataset = await collection.findOne({ _id: new ObjectID(id) });

  if (!dataset) {
    res.send({
      code: 404,
      data: "Not found",
    });
    return;
  }

  return res.send({
    code: 200,
    data: {
      id: dataset._id.toHexString(),
      ...dataset,
    },
  });
});

export default router;
