import express from "express";
import mongodb, { ObjectId } from "mongodb";
import Annotation from "../models/Annotation";
import DatasetGroup from "../models/DatasetGroup";

type Axis = "x" | "y" | "z";

function validate(axis: string, index: number): string {
  if (axis !== "z" && axis !== "y" && axis !== "x") {
    return "invalid axis";
  }

  if (isNaN(index) || index < 0) {
    return "invalid index";
  }

  return "";
}

const router = express.Router();

/**
 * get annotations of dataset slice in a group
 */
router.get("/:group/:dataset/:axis/:index", async (req, res) => {
  const { group: groupId, dataset: datasetId, axis } = req.params;
  const index = parseInt(req.params.index);

  const error = validate(axis, index);
  if (error.length > 0) {
    res.send({
      code: 400,
      data: error,
    });
    return;
  }

  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("groups");

  const group: DatasetGroup = await collection.findOne({
    _id: new ObjectId(groupId),
  });

  if (!group) {
    res.send({
      code: 404,
      data: `group ${datasetId} not found`,
    });
    return;
  }

  const result: Annotation[] = [];
  group.labels.forEach((label) => {
    const annotationsOfDataset = label.annotations[datasetId];
    if (!annotationsOfDataset) {
      return;
    }

    const annotationsOfSlice = annotationsOfDataset[axis as Axis][index];
    if (!annotationsOfSlice) return;

    result.concat(annotationsOfSlice);
  });

  res.send({
    code: 200,
    data: result,
  });
});

/**
 * add annotations of dataset slice in a group
 */
router.post("/:group/:dataset/:axis/:index", async (req, res) => {
  const { group: groupId, dataset: datasetId, axis } = req.params;
  const annotations: Annotation[] = req.body;
  const index = parseInt(req.params.index);

  const error = validate(axis, index);
  if (error.length > 0) {
    res.send({
      code: 400,
      data: error,
    });
    return;
  }

  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("groups");

  await collection.updateOne(
    {
      _id: new ObjectId(groupId),
    },
    {
      $push: {
        [`annotations.${datasetId}.${axis}.${index}`]: {
          $each: annotations.map(({ tag, type, coordinates, comment }) => ({
            tag,
            type,
            coordinates,
            comment,
          })),
        },
      },
    }
  );

  res.send({
    code: 200,
  });
});

/* router.post("/:dataset/:axis/:index/:id", async (req, res) => {
  const { dataset, axis, id } = req.params;
  const { comment, coordinates } = req.body;
  const index = parseInt(req.params.index);

  const error = validate(axis, index);
  if (error.length > 0) {
    res.send({
      code: 400,
      data: error,
    });
    return;
  }

  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("datasets");

  const item = await collection.findOne({
    _id: dataset
  });

  if (!item) {
    res.send({
      code: 404,
      data: "dataset not found.",
    });
    return;
  }

  if (comment) {
    item.comment = comment;
  }

  if (coordinates) {
    item.coordinates = JSON.stringify(coordinates);
  }

  await item.save();

  res.send({
    code: 200,
  });
}); */

export default router;
