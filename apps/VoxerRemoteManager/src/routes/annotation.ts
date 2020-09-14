import express from "express";
import mongodb, { ObjectId } from "mongodb";
import DatasetGroup, { Label } from "../models/DatasetGroup";

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
router.get("/:groupId/:datasetId/:axis/:index", async (req, res) => {
  const { groupId, datasetId, axis } = req.params;
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
      data: `group ${groupId} not found`,
    });
    return;
  }

  const dataset = group.datasets[datasetId];
  if (!dataset) {
    res.send({
      code: 404,
      data: `dataset ${datasetId} not found`,
    });
    return;
  }

  const labelMap: Record<string, Label> = {};
  group.labels.forEach((label) => {
    labelMap[label.id.toHexString()] = label;
  });

  const result: any[] = [];
  Object.entries(dataset.labels).forEach(([labelId, annotations]) => {
    const annotationsOfSlice = annotations[axis as Axis][index];
    if (!annotationsOfSlice) return;

    const label = labelMap[labelId];

    if (!label) return;

    result.concat({
      tag: labelId,
      type: label.type,
      ...annotationsOfSlice,
    });
  });

  res.send({
    code: 200,
    data: result,
  });
});

/**
 * add an annotation of dataset slice in a group
 */
router.post("/:group/:dataset/:axis/:index", async (req, res) => {
  const { group: groupId, dataset: datasetId, axis } = req.params;
  const { tag, comment, coordinates } = req.body;
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
        [`datasets.${datasetId}.labels.${tag}.${axis}.${index}`]: {
          comment,
          coordinates,
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
