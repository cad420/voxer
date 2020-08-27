import express from "express";
import Pipeline from "../models/Pipeline";
import { Scene } from "../models/voxer";

const router = express.Router();

router.get("/", async (req, res) => {
  const pipelines = await Pipeline.findAll();
  res.send({
    code: 200,
    data: Object.values(pipelines),
  });
});

router.post("/", async (req, res) => {
  const scene = req.body as Scene;

  // TODO: validate scene params
  // TODO: allow `comment`
  const pipeline = await Pipeline.create({
    params: JSON.stringify(scene),
  });

  res.send({
    code: 200,
    data: pipeline.id,
  });
});

router.get("/:id", async (req, res) => {
  const { id } = req.params;
  const pipeline = await Pipeline.findOne({
    where: {
      id: parseInt(id),
    },
  });

  if (!pipeline) {
    res.send({
      code: 404,
      data: "Not Found",
    });
    return;
  }

  res.send({
    code: 200,
    data: JSON.parse(pipeline.params),
  });
});

router.put("/:id", async (req, res) => {
  const { id } = req.params;
  const params = req.body;

  // TODO: validate scene params

  const pipeline = await Pipeline.findOne({ where: { id } });

  if (!pipeline) {
    res.send({
      code: 404,
      data: "Pipeline not found.",
    });
    return;
  }

  pipeline.params = JSON.stringify(params);
  await pipeline.save();

  res.send({
    code: 200,
  });
});

router.delete("/:id", async (req, res) => {
  const { id } = req.params;
  await Pipeline.destroy({
    where: { id },
  });

  res.send({
    code: 200,
  });
});

export default router;
