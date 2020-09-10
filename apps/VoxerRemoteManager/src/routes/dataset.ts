import express from "express";
import multer from "multer";
import mongodb, { ObjectId } from "mongodb";
import { resolve } from "path";
import Dataset from "../models/Dataset";
import { PUBLIC_PATH, UPLOAD_PATH } from "../config";
import messager from "../messager";

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

router.get("/", async (req, res) => {
  const database: mongodb.Db = req.app.get("database");

  const collection = database.collection("datasets");
  const datasets = await collection.find({}).toArray();

  res.send({
    code: 200,
    data: datasets.map((dataset: Dataset) => {
      const info = messager.cache[dataset._id.toString()];
      return {
        id: dataset._id,
        name: dataset.name,
        dimensions: info ? info.dimensions : [],
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

router.get("/:id", async (req, res) => {
  const { id } = req.params;
  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("datasets");

  const dataset: Dataset = await collection.findOne({ _id: new ObjectId(id) });

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
