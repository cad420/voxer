import express from "express";
import mongodb, { ObjectID } from "mongodb";
import Pipeline from "../models/Pipeline";

const router = express.Router();

router.get("/", async (req, res) => {
  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("pipelines");

  const pipelines: Pipeline[] = await collection.find().project({ comment: true, type: true }).toArray();
  res.send({
    code: 200,
    data: pipelines.map(pipeline => ({
      id: pipeline._id.toHexString(),
      comment: pipeline.comment,
      type: pipeline.type
    })),
  });
});

router.post("/", async (req, res) => {
  const params = req.body;
  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("pipelines");

  // TODO: validate scene params
  // TODO: allow `comment`
  delete params.id;
  delete params._id;
  const result = await collection.insertOne({
    ...params,
  });

  res.send({
    code: 200,
    data: result.insertedId,
  });
});

router.get("/:id", async (req, res) => {
  const { id } = req.params;
  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("pipelines");

  const pipeline = await collection.findOne({
    _id: new ObjectID(id),
  });

  if (!pipeline) {
    res.send({
      code: 404,
      data: "Not Found",
    });
    return;
  }

  pipeline.id = pipeline._id;
  
  res.send({
    code: 200,
    data: pipeline,
  });
});

router.put("/:id", async (req, res) => {
  const { id } = req.params;
  const params = req.body;
  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("pipelines");

  // TODO: validate scene params

  const pipeline = await collection.findOne({ _id: new ObjectID(id) });

  if (!pipeline) {
    res.send({
      code: 404,
      data: "Pipeline not found.",
    });
    return;
  }

  delete params.id;
  delete params._id;
  await collection.updateOne(
    {
      _id: new ObjectID(id),
    },
    {
      $set: params,
    }
  );

  res.send({
    code: 200,
  });
});

router.delete("/:id", async (req, res) => {
  const { id } = req.params;
  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("pipelines");

  await collection.deleteOne({ id });

  res.send({
    code: 200,
  });
});

export default router;
