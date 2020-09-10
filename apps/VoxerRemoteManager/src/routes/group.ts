import express from "express";
import mongodb, { ObjectId } from "mongodb";
import DatasetGroup from "../models/DatasetGroup";
import Dataset from "../models/Dataset";

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

  const result = await collection.findOne(
    { id: new ObjectId(id) },
    {
      fields: {
        "labels.annotations": 0,
      },
    }
  );

  res.send({
    code: 200,
    data: result,
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
          annotations: {},
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
  const { id } = req.params;

  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("groups");

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

  // TODO: make sure dataset not exist in group
  const result = await collection.updateOne(
    { _id: new ObjectId(id) },
    {
      $push: {
        datasets: {
          id: new ObjectId(id),
          name: dataset.name,
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
