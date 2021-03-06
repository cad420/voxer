import express from "express";
import mongodb, { ObjectID } from "mongodb";
import DatasetGroup, { Label } from "../models/DatasetGroup";
import Annotation from "../models/Annotation";

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
  const collection = database.collection<DatasetGroup>("groups");

  const group = (await collection.findOne(
    {
      _id: new ObjectID(groupId),
    },
    {
      projection: {
        _id: false,
        id: {
          $toString: "$_id",
        },
        name: true,
        labels: true,
        datasets: true,
      },
    }
  )) as DatasetGroup;

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

  type ResAnnotation = {
    tag: string;
    type: string;
    coordinates: Annotation["coordinates"];
    comment: string;
  };
  const result: ResAnnotation[] = [];
  Object.entries(dataset.labels).forEach(([labelId, annotations]) => {
    const axisOfAnnotations = annotations[axis as Axis];
    if (!axisOfAnnotations) return;

    const annotationsOfSlice = axisOfAnnotations[index.toString()];
    if (!annotationsOfSlice) return;

    const label = labelMap[labelId];

    if (!label) return;

    annotationsOfSlice.forEach((item) => {
      result.push({
        type: label.type,
        tag: labelId,
        ...item,
      });
    });
  });

  res.send({
    code: 200,
    data: result,
  });
});

async function saveAnnotations(
  database: mongodb.Db,
  groupId: string,
  dataset: string,
  axis: string,
  index: number,
  annotations: Array<{
    tag: string;
    comment: string;
    coordinates: Annotation["coordinates"];
  }>
) {
  const collection = database.collection("groups");
  const group = await collection.findOne(
    {
      _id: new ObjectID(groupId),
    },
    {
      projection: {
        _id: true,
        labels: true,
      },
    }
  );

  const updateExp: Record<string, {}> = {};
  (group.labels as DatasetGroup["labels"]).forEach((label) => {
    const data = annotations.filter(
      (annotation) => annotation.tag === label.id.toHexString()
    );

    updateExp[`datasets.${dataset}.labels.${label.id}.${axis}.${index}`] = data;
  });
  await collection.updateOne(
    {
      _id: group._id,
    },
    {
      $set: updateExp,
    }
  );
}

/**
 * set annotations of a dataset slice in a group
 */
router.post("/:group/:dataset/:axis/:index", async (req, res) => {
  const { group: groupId, dataset: datasetId, axis } = req.params;

  // TODO: validate req.body
  type ReqData = Array<{
    tag: string;
    comment: string;
    coordinates: Annotation["coordinates"];
  }>;
  const annotations = req.body as ReqData;
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
  await saveAnnotations(database, groupId, datasetId, axis, index, annotations);

  res.send({
    code: 200,
  });
});

export default router;
