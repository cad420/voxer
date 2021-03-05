import { FastifyInstance } from "fastify";
import { ObjectID } from "mongodb";
import Label from "../../models/Labels";

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

async function routes(server: FastifyInstance) {
  const db = server.mongo.db;
  if (!db) {
    return;
  }

  /**
   * get annotations of dataset slice in a group
   */
  // server.get("/groups/:groupId/:datasetId/:axis/:index", async (req, res) => {
  //   const { groupId, datasetId, axis } = req.params;
  //   const index = parseInt(req.params.index);

  //   const error = validate(axis, index);
  //   if (error.length > 0) {
  //     res.send({
  //       code: 400,
  //       data: error,
  //     });
  //     return;
  //   }

  //   const db: mongodb.Db = req.app.get("db");
  //   const collection = db.collection<DatasetGroup>("groups");

  //   const group = (await collection.findOne(
  //     {
  //       _id: new ObjectID(groupId),
  //     },
  //     {
  //       projection: {
  //         _id: false,
  //         id: {
  //           $toString: "$_id",
  //         },
  //         name: true,
  //         createTime: true,
  //         creator: true,
  //         labels: true,
  //         datasets: true,
  //         users: true,
  //         applications: true,
  //       },
  //     }
  //   )) as DatasetGroup;

  //   if (!group) {
  //     res.send({
  //       code: 404,
  //       data: `group ${groupId} not found`,
  //     });
  //     return;
  //   }

  //   const dataset = group.datasets[datasetId];
  //   if (!dataset) {
  //     res.send({
  //       code: 404,
  //       data: `dataset ${datasetId} not found`,
  //     });
  //     return;
  //   }

  //   const labelMap: Record<string, Label> = {};
  //   group.labels.forEach((label) => {
  //     labelMap[label.id.toHexString()] = label;
  //   });

  //   type ResAnnotation = {
  //     tag: string;
  //     type: string;
  //     coordinates: Annotation["coordinates"];
  //     comment: string;
  //   };
  //   const result: ResAnnotation[] = [];
  //   Object.entries(dataset.labels).forEach(([labelId, annotations]) => {
  //     const axisOfAnnotations = annotations[axis as Axis];
  //     if (!axisOfAnnotations) return;

  //     const annotationsOfSlice = axisOfAnnotations[index.toString()];
  //     if (!annotationsOfSlice) return;

  //     const label = labelMap[labelId];

  //     if (!label) return;

  //     annotationsOfSlice.forEach((item) => {
  //       result.push({
  //         type: label.type,
  //         tag: labelId,
  //         ...item,
  //       });
  //     });
  //   });

  //   res.send({
  //     code: 200,
  //     data: result,
  //   });
  // });

  // async function saveAnnotations(
  //   db: mongodb.Db,
  //   groupId: string,
  //   dataset: string,
  //   axis: string,
  //   index: number,
  //   annotations: Array<{
  //     tag: string;
  //     comment: string;
  //     coordinates: Annotation["coordinates"];
  //   }>
  // ) {
  //   const collection = db.collection("groups");
  //   const group = await collection.findOne(
  //     {
  //       _id: new ObjectID(groupId),
  //     },
  //     {
  //       projection: {
  //         _id: true,
  //         labels: true,
  //       },
  //     }
  //   );

  //   const updateExp: Record<string, {}> = {};
  //   (group.labels as DatasetGroup["labels"]).forEach((label) => {
  //     const data = annotations.filter(
  //       (annotation) => annotation.tag === label.id.toHexString()
  //     );

  //     updateExp[`datasets.${dataset}.labels.${label.id}.${axis}.${index}`] = data;
  //   });
  //   await collection.updateOne(
  //     {
  //       _id: group._id,
  //     },
  //     {
  //       $set: updateExp,
  //     }
  //   );
  // }

  /**
   * set annotations of a dataset slice in a group
   */
  // router.post("/:group/:dataset/:axis/:index", async (req, res) => {
  //   const { group: groupId, dataset: datasetId, axis } = req.params;

  //   // TODO: validate req.body
  //   type ReqData = Array<{
  //     tag: string;
  //     comment: string;
  //     coordinates: Annotation["coordinates"];
  //   }>;
  //   const annotations = req.body as ReqData;
  //   const index = parseInt(req.params.index);

  //   const error = validate(axis, index);
  //   if (error.length > 0) {
  //     res.send({
  //       code: 400,
  //       data: error,
  //     });
  //     return;
  //   }

  //   const db: mongodb.Db = req.app.get("db");
  //   await saveAnnotations(db, groupId, datasetId, axis, index, annotations);

  //   res.send({
  //     code: 200,
  //   });
  // });

  /**
   * update a label for a group
   */
  // server.put("/:groupId/labels/:labelId", async (req, res) => {
  //   // TODO: validate body
  //   const newLabel = req.body;
  //   const { groupId, labelId } = req.params;

  //   const db: mongodb.Db = req.app.get("db");
  //   const collection = db.collection("groups");

  //   const group = await collection.findOne(
  //     { _id: new ObjectId(groupId) },
  //     {
  //       projection: {
  //         _id: false,
  //         id: {
  //           $toString: "$_id",
  //         },
  //         labels: true,
  //       },
  //     }
  //   );

  //   if (!group) {
  //     res.send({
  //       code: 404,
  //       data: "group not found",
  //     });
  //     return;
  //   }

  //   const labels = group.labels as DatasetGroup["labels"];
  //   let idx = -1;
  //   let id = new ObjectID();
  //   for (let i = 0; i < labels.length; ++i) {
  //     if (labels[i].id.toHexString() === labelId) {
  //       idx = i;
  //       id = labels[i].id;
  //       break;
  //     }
  //   }

  //   if (idx === -1) {
  //     res.send({
  //       code: 404,
  //       data: "label not found",
  //     });
  //     return;
  //   }

  //   const result = await collection.updateOne(
  //     { _id: new ObjectId(groupId) },
  //     {
  //       $set: {
  //         [`labels.${idx}`]: {
  //           ...newLabel,
  //           id,
  //         },
  //       },
  //     }
  //   );

  //   if (result.modifiedCount !== 1) {
  //     res.send({
  //       code: 400,
  //       data: "Failed to update",
  //     });
  //     return;
  //   }

  //   res.send({
  //     code: 200,
  //   });
  // });
}

export default routes;
