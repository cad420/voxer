import express from "express";
import mongodb, { ObjectID } from "mongodb";
import { resolve, basename } from "path";
import { UPLOAD_PATH } from "../config";
import fs from "fs-extra";
import Dataset from "../models/Dataset";
import { getDatasetInfo } from "../worker_api/jsonrpc";

const router = express.Router();

async function createPipelineAndSave(
  filename: string,
  database: mongodb.Db
): Promise<string> {
  let collection = database.collection("datasets");
  const dataset = (await collection.findOne(
    { path: filename },
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

  let id = "";
  let maxDim = 300;
  if (dataset) {
    id = dataset.id;
    maxDim = Math.max(dataset.dimensions[0], dataset.dimensions[1]);
    console.log(`Autom data already exist: ${id}`);
  } else {
    const result = await collection.insertOne({
      path: filename,
      name: filename,
      dimensions: [1, 1, 1],
      histogram: [],
      range: [1, 1],
    });
    id = result.insertedId.toHexString();

    try {
      const info = await getDatasetInfo(id, filename, filename);
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
      maxDim = Math.max(info.dimensions[0], info.dimensions[1]);
      console.log(`Autom data uploaded: ${id}`);
    } catch (err) {
      await collection.deleteOne({ _id: new ObjectID(id) });
      throw new Error(err.message);
    }
  }

  collection = database.collection("pipelines");
  const exist = await collection.findOne(
    {
      "volumes.0.dataset": id,
    },
    {
      projection: {
        _id: true,
      },
    }
  );

  if (exist) {
    console.log(`Autom data's pipeline already exist: ${exist._id}`);
    return (exist._id as ObjectID).toHexString();
  }

  const pipeline: any = {
    type: "et",
    comment: filename,
    tfcns: [
      [
        { x: 0, y: 0, color: "#AAAAAA" },
        { x: 1, y: 1, color: "#AAAAAA" },
      ],
    ],
    volumes: [{ dataset: id, spacing: [1, 1, 1], tfcn: 0, render: true }],
    isosurfaces: [],
    camera: {
      width: 400,
      height: 400,
      pos: [0.0, 0.0, maxDim * 1.5],
      target: [0.0, 0.0, 0.0],
      up: [0.0, 1.0, 0.0],
      zoom: 1,
    },
  };
  const res = await collection.insertOne(pipeline);
  console.log(`Autom data's pipeline created: ${res.insertedId}`);

  return res.insertedId;
}

router.post("/", async (req, res) => {
  const { path } = req.body;
  const database: mongodb.Db = req.app.get("database");

  const stat = await fs.stat(path);
  if (!stat || !stat.isFile) {
    return res.send({
      code: 404,
      data: "Dataset Not Found.",
    });
  }

  const filename = basename(path);
  const destFilepath = resolve(UPLOAD_PATH, filename);
  await fs.copyFile(path, destFilepath);
  const id = await createPipelineAndSave(filename, database);

  return res.send({
    code: 200,
    data: id,
  });
});

export default router;
