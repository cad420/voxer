import express from "express";
import multer from "multer";
import mongodb, { ObjectID } from "mongodb";
import { options } from "../index";
import Dataset from "../models/Dataset";
import WorkerRPCCaller from "../worker_api";
import { ResBody } from "./index";
import DatasetGroup from "../models/DatasetGroup";

const router = express.Router();

const storage = multer.diskStorage({
  destination: (req, file, cb) => {
    cb(null, options.storage);
  },
  filename: (req, file, cb) => {
    cb(null, file.originalname);
  },
});
export const upload = multer({ storage });

/**
 * get all datasets
 */
router.get<{}, ResBody>("/", async (req, res) => {
  const db: mongodb.Db = req.app.get("database");
  const datasets = db.collection("datasets");

  let page = parseInt(req.query.page);
  if (isNaN(page) || page <= 0) {
    page = 1;
  }

  let size = parseInt(req.query.size);
  if (isNaN(page) || size <= 0) {
    size = 10;
  }

  const total = await datasets.countDocuments();
  const list = await datasets
    .find(
      {},
      {
        projection: {
          _id: false,
          id: {
            $toString: "$_id",
          },
          name: true,
          dimensions: true,
          range: true,
        },
      }
    )
    .skip((page - 1) * size)
    .limit(size)
    .toArray();

  res.send({
    code: 200,
    data: {
      list,
      total,
    },
  });
});

/**
 * add a dataset by upload
 */
router.post("/", upload.single("dataset"), async (req, res) => {
  let path = "";
  const { name } = req.body;
  if (req.file) {
    path = req.file.filename;
  } else {
    path = req.body.path;
  }

  const db: mongodb.Db = req.app.get("database");
  const worker: WorkerRPCCaller = req.app.get("worker");
  const collection = db.collection("datasets");

  const exist = (await collection.findOne(
    { path },
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
      path, // TODO
      name,
      dimensions: [1, 1, 1],
      histogram: [],
      range: [1, 1],
      groups: [],
    });
    id = result.insertedId;
  }

  try {
    const info = await worker.getDatasetInfo(id, name, path);
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

/**
 * get a dataset info
 */
router.get("/:id", async (req, res) => {
  const { id } = req.params;

  const database: mongodb.Db = req.app.get("database");
  const worker: WorkerRPCCaller = req.app.get("worker");

  const collection = database.collection<Dataset>("datasets");

  const dataset = await collection.findOne<Dataset>(
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
  );

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

/**
 * delete a dataset
 */
router.delete<{ id: string }, ResBody>("/:id", async (req, res) => {
  const { id } = req.params;
  const datasetId = new ObjectID(id);
  const db: mongodb.Db = req.app.get("database");
  const datasets = db.collection<Dataset>("datasets");
  const result = await datasets.findOneAndDelete({
    _id: new ObjectID(id),
  });
  if (!result.value) {
    res.send({
      code: 404,
      data: "Dataset not found",
    });
    return;
  }
  if (!result.ok) {
    res.send({
      code: 500,
      data: "Failed to delete dataset",
    });
    return;
  }

  const dataset = result.value;
  const groups = db.collection<DatasetGroup>("datasets");
  const op = await groups.updateMany(
    { _id: { $in: dataset.groups || [] } },
    {
      $pull: {
        datasets: datasetId,
      },
    }
  );

  if (!op.result.ok) {
    res.send({
      code: 500,
      data: "Failed to delete dataset",
    });
    return;
  }

  res.send({ code: 200 });
});

export default router;
