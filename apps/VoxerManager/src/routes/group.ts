import express from "express";
import mongodb, { ObjectId, ObjectID } from "mongodb";
import DatasetGroup from "../models/DatasetGroup";
import Dataset from "../models/Dataset";
import { DatasetAnnotations } from "../models/Annotation";

const router = express.Router();

/**
 * get all groups
 */
router.get("/", async (req, res) => {
  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("groups");

  const groups = await collection
    .find(
      {},
      {
        projection: {
          _id: false,
          id: {
            $toString: "$_id",
          },
          name: true,
        },
      }
    )
    .toArray();

  res.send({
    code: 200,
    data: groups,
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
      {
        $project: {
          _id: false,
          id: {
            $toString: "$_id",
          },
          name: true,
          labels: true,
          datasets: { $objectToArray: "$datasets" },
        },
      },
      { $project: { "datasets.v.labels": 0 } },
      {
        $project: {
          id: true,
          datasets: { $arrayToObject: "$datasets" },
          name: true,
          labels: true,
        },
      },
    ])
    .toArray();

  if (result.length === 0) {
    res.send({
      code: 404,
      data: "group not found.",
    });
    return;
  }

  const datasetCollection = database.collection("datasets");
  const group = { ...result[0] };
  const tasks = Object.keys(group.datasets || []).map(async (id) => {
    const dataset = await datasetCollection.findOne(
      { _id: new ObjectID(id) },
      {
        projection: {
          _id: false,
          id: {
            $toString: "$_id",
          },
          name: true,
          dimensions: true,
        },
      }
    );
    return dataset;
  });
  const datasets = await Promise.all(tasks);

  const labels = (group.labels || []).map((item) => ({
    ...item,
    id: item.id.toHexString(),
  }));

  res.send({
    code: 200,
    data: {
      id: group.id,
      name: group.name,
      labels: labels,
      datasets: datasets,
    },
  });
});

/**
 * add a label for a group
 */
router.post("/:groupId/labels", async (req, res) => {
  // TODO: validate body
  const label = req.body;
  const { groupId } = req.params;

  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("groups");

  const labelId = new ObjectID();
  const result = await collection.updateOne(
    { _id: new ObjectId(groupId) },
    {
      $push: {
        labels: {
          id: labelId,
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
    data: labelId.toHexString(),
  });
});

/**
 * remove a label for a group
 */
router.delete("/:groupId/labels/:labelId", async (req, res) => {
  // TODO: validate body
  const { groupId, labelId } = req.params;

  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("groups");

  const group = await collection.findOne(
    {
      _id: new ObjectId(groupId),
    },
    {
      projection: {
        labels: true,
        datasets: true,
      },
    }
  );

  if (!group) {
    res.send({
      code: 404,
      data: "group not found",
    });
    return;
  }

  // TODO: too much overhead
  Object.values(group.datasets).forEach((item) => {
    if (item.labels[labelId]) {
      delete item.labels[labelId];
    }
  });

  const result = await collection.updateOne(
    { _id: new ObjectId(groupId) },
    {
      $pull: {
        labels: {
          id: new ObjectID(labelId),
        },
      },
      $set: {
        datasets: group.datasets,
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
 * update a label for a group
 */
router.put("/:groupId/labels/:labelId", async (req, res) => {
  // TODO: validate body
  const newLabel = req.body;
  const { groupId, labelId } = req.params;

  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("groups");

  const group = await collection.findOne(
    { _id: new ObjectId(groupId) },
    {
      projection: {
        _id: false,
        id: {
          $toString: "$_id",
        },
        labels: true,
      },
    }
  );

  if (!group) {
    res.send({
      code: 404,
      data: "group not found",
    });
    return;
  }

  const labels = group.labels as DatasetGroup["labels"];
  let idx = -1;
  let id = new ObjectID();
  for (let i = 0; i < labels.length; ++i) {
    if (labels[i].id.toHexString() === labelId) {
      idx = i;
      id = labels[i].id;
      break;
    }
  }

  if (idx === -1) {
    res.send({
      code: 404,
      data: "label not found",
    });
    return;
  }

  const result = await collection.updateOne(
    { _id: new ObjectId(groupId) },
    {
      $set: {
        [`labels.${idx}`]: {
          ...newLabel,
          id,
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

  const dataset: Dataset = await database.collection("datasets").findOne(
    {
      _id: new ObjectId(datasetId),
    },
    {
      projection: {},
    }
  );

  if (!dataset) {
    res.send({
      code: 404,
      data: "dataset not found.",
    });
    return;
  }

  const collection = database.collection("groups");

  const group = await collection.findOne(
    {
      _id: new ObjectID(groupId),
    },
    {
      projection: {
        datasets: true,
        labels: true,
      },
    }
  );

  if (!group) {
    res.send({
      code: 404,
      data: "group not found.",
    });
    return;
  }

  if ((group.datasets as DatasetGroup["datasets"])[datasetId]) {
    res.send({
      code: 200,
    });
    return;
  }

  const labels: Record<string, DatasetAnnotations> = {};
  (group.labels as DatasetGroup["labels"]).forEach((label) => {
    labels[label.id.toHexString()] = { z: {}, y: {}, x: {} };
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

export default router;
