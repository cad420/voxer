import express from "express";
import mongodb, { ObjectId } from "mongodb";
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

  type ResAnnotation = {
    tag: string;
    type: string;
    coordinates: Annotation["coordinates"];
    comment: string;
  };
  const result: ResAnnotation[] = [];
  Object.entries(dataset.labels).forEach(([labelId, annotations]) => {
    const annotationsOfSlice = annotations[axis as Axis][index.toString()];
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

/**
 * set annotations of a dataset slice in a group
 */
router.post("/:group/:dataset/:axis/:index", async (req, res) => {
  const { group: groupId, dataset: datasetId, axis } = req.params;

  // TODO: validate req.body
  type ReqAnnotation = {
    tag: string;
    comment: string;
    coordinates: Annotation["coordinates"];
  };
  const list: ReqAnnotation[] = req.body;
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

  const updateExp: Record<string, Array<Annotation>> = {};
  list.map((annotation) => {
    const { tag } = annotation;
    const key = `datasets.${datasetId}.labels.${tag}.${axis}.${index}`;

    if (updateExp[key]) {
      updateExp[key].push(annotation);
    } else {
      updateExp[key] = [
        annotation,
      ];
    }
  });

  await collection.updateOne(
    {
      _id: new ObjectId(groupId),
    },
    {
      $set: list.length === 0 ? {
        [`datasets.${datasetId}.labels`]: {}
      } : updateExp,
    }
  );

  res.send({
    code: 200,
  });
});

export default router;
