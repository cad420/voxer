import express from "express";
import mongodb, { ObjectId, ObjectID } from "mongodb";
import DatasetGroup, { LabelId } from "../models/DatasetGroup";
import Dataset from "../models/Dataset";
import { DatasetAnnotations } from "../models/Annotation";

const router = express.Router();

/**
 * get all groups
 */
router.get("/", async (req, res) => {
  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("groups");

  const groups: DatasetGroup[] = await collection.find().toArray();

  res.send({
    code: 200,
    data: groups.map((item) => ({
      id: item._id.toHexString(),
      name: item.name,
    })),
  });
});

/**
 * add group
 */
router.post("/", async (req, res) => {
  const { name } = req.body;

  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("groups");

  const result = await collection.insertOne({
    name,
    labels: [],
    datasets: [],
  });

  res.send({
    code: 200,
    data: result.insertedId,
  });
});

/**
 * get group info
 */
router.get("/:id", async (req, res) => {
  const { id } = req.params;

  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("groups");
  const result: DatasetGroup[] = await collection
    .aggregate([
      { $match: { _id: new ObjectId(id) } },
      { $project: { datasets: { $objectToArray: "$datasets" } } },
      { $project: { "datasets.v.labels": 0 } },
      { $project: { datasets: { $arrayToObject: "$datasets" } } },
    ])
    .toArray();

  if (result.length === 0) {
    res.send({
      code: 404,
      data: "group not found.",
    });
    return;
  }

  res.send({
    code: 200,
    data: result[0],
  });
});

/**
 * add a label for a group
 */
router.post("/:id/labels", async (req, res) => {
  // TODO: validate body
  const label = req.body;
  const { id } = req.params;

  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("groups");

  const result = await collection.updateOne(
    { _id: new ObjectId(id) },
    {
      $push: {
        labels: {
          name: label.name,
          color: label.color,
          type: label.type,
        },
      },
    }
  );

  if (result.modifiedCount !== 1) {
    res.send({
      code: 400,
      data: "Failed to update",
    });
    return;
  }

  res.send({
    code: 200,
  });
});

/**
 * add a dataset for a group
 */
router.post("/:id/datasets", async (req, res) => {
  // TODO: validate body
  const { id: datasetId } = req.body;
  const { id: groupId } = req.params;

  const database: mongodb.Db = req.app.get("database");

  const dataset: Dataset = await database.collection("datasets").findOne({
    _id: new ObjectId(datasetId),
  });

  if (!dataset) {
    res.send({
      code: 404,
      data: "dataset not found.",
    });
    return;
  }

  const collection = database.collection("groups");

  const group: DatasetGroup = await collection.findOne({
    _id: new ObjectID(groupId),
  });

  if (!group) {
    res.send({
      code: 404,
      data: "group not found.",
    });
    return;
  }

  if (group.datasets[datasetId]) {
    res.send({
      code: 200,
    });
    return;
  }

  const labels: Record<LabelId, DatasetAnnotations> = {};
  group.labels.forEach((label) => {
    labels[label.id] = { z: {}, y: {}, x: {} };
  });

  await collection.updateOne(
    { _id: new ObjectId(groupId) },
    {
      [`datasets.${datasetId}`]: {
        name: dataset.name,
        labels,
      },
    }
  );

  res.send({
    code: 200,
  });
});
