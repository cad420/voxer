import express from "express";
import multer from "multer";
import mongodb, { ObjectID } from "mongodb";
import { options } from "../index";
import Dataset from "../models/Dataset";
import WorkerRPCCaller from "../worker_api";

const router = express.Router();

const storage = multer.diskStorage({
  destination: (req, file, cb) => {
    cb(null, options.storage);
  },
  filename: (req, file, cb) => {
    cb(null, file.originalname);
  },
});
const upload = multer({ storage });

router.get("/", async (req, res) => {
  const database: mongodb.Db = req.app.get("database");

  const collection = database.collection("datasets");
  const datasets = await collection
    .find(
      {},
      {
        projection: {
          _id: false,
          id: {
            $toString: "$_id",
          },
          name: true,
          path: true,
          dimensions: true,
          histogram: true,
          range: true,
        },
      }
    )
    .toArray();

  res.send({
    code: 200,
    data: datasets,
  });
});

router.post("/", upload.single("dataset"), async (req, res) => {
  const filename = req.file.filename;

  const database: mongodb.Db = req.app.get("database");
  const worker: WorkerRPCCaller = req.app.get("worker");
  const collection = database.collection("datasets");

  const exist = (await collection.findOne(
    { path: filename },
    {
      projection: {
        id: {
          $toString: "$_id",
        },
        histogram: true,
      },
    }
  )) as Dataset;
  let id = "";
  if (exist) {
    id = exist.id;
    if (exist.histogram && exist.histogram.length > 0) {
      res.send({
        code: 200,
        data: id,
      });
      return;
    }
  } else {
    const result = await collection.insertOne({
      path: filename, // TODO
      name: filename,
      dimensions: [1, 1, 1],
      histogram: [],
      range: [1, 1],
    });
    id = result.insertedId;
  }

  try {
    const info = await worker.getDatasetInfo(id, filename, filename);
    await collection.updateOne(
      { _id: new ObjectID(id) },
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
  } catch (err) {
    await collection.deleteOne({ _id: new ObjectID(id) });
    res.send({
      code: 400,
      data: err.message,
    });
  }
});

router.get("/:id", async (req, res) => {
  const { id } = req.params;

  const database: mongodb.Db = req.app.get("database");
  const worker: WorkerRPCCaller = req.app.get("worker");

  const collection = database.collection("datasets");

  const dataset = (await collection.findOne(
    { _id: new ObjectID(id) },
    {
      projection: {
        _id: false,
        id: {
          $toString: "$_id",
        },
        name: true,
        path: true,
        dimensions: true,
        histogram: true,
        range: true,
      },
    }
  )) as Dataset;

  if (dataset.histogram.length === 0) {
    try {
      const info = await worker.getDatasetInfo(
        dataset.id,
        dataset.name,
        dataset.path
      );
      await collection.updateOne(
        { _id: new ObjectID(dataset.id) },
        {
          $set: {
            dimensions: info.dimensions,
            histogram: info.histogram,
            range: info.range,
          },
        }
      );
    } catch (err) {
      res.send({
        code: 500,
        data: err.message,
      });
      return;
    }
  }

  if (!dataset) {
    res.send({
      code: 404,
      data: "Not found",
    });
    return;
  }

  return res.send({
    code: 200,
    data: dataset,
  });
});

export default router;
