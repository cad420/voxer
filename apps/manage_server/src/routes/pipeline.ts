import express from "express";
import store from "../models/Pipeline";
import { Pipeline } from "../models/voxer";

const router = express.Router();

router.get("/", async (req, res) => {
  const pipelines = await store.getAll();
  res.send({
    code: 200,
    data: Object.values(pipelines)
  });
});

router.post("/", async (req, res) => {
  const pipeline = req.body as Pipeline;
  const id = await store.add(pipeline);

  res.send({
    code: 200,
    data: id
  });
});

router.get("/:id", async (req, res) => {
  const { id } = req.params;
  const pipeline = await store.get(id);
  res.send({
    code: 200,
    data: pipeline
  });
});

router.put("/:id", async (req, res) => {
  const { id } = req.params;
  const params = req.body;
  const pipeline = await store.get(id);

  if (!pipeline) {
    res.send({
      code: 404,
      data: "Pipeline not found."
    });
  }

  await store.update(id, { id, params: params as Pipeline["params"] });

  res.send({
    code: 200
  });
});

// router.delete("/:id", () => {});

export default router;
