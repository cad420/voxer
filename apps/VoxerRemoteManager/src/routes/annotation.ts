import express from "express";
import Annotation from "../models/Annotation";

const router = express.Router();

router.get("/:dataset/:axis/:index", async (req, res) => {
  const { dataset, axis, index } = req.params;
  const annotations = await Annotation.findAll({
    where: {
      dataset: parseInt(dataset),
      axis,
      index: parseInt(index),
    },
  });

  res.send({
    code: 200,
    data: annotations.map(({ id, tag, comment, type, coordinates }) => ({
      id,
      tag,
      comment,
      type,
      coordinates,
    })),
  });
});

router.post("/:dataset/:axis/:index", async (req, res) => {
  const { dataset, axis, index } = req.params;
  const { tag, type, coordinates, comment = '' } = req.body;
  const annotation = await Annotation.create({
    dataset: parseInt(dataset),
    axis,
    index: parseInt(index),
    tag,
    comment,
    type,
    coordinates: JSON.stringify(coordinates)
  });

  res.send({
    code: 200,
    data: annotation.id
  })
});

router.post("/:annotation", async (req, res) => {
  const { annotation } = req.params;
  const { comment, coordinates } = req.body;

  const item = await Annotation.findOne({
    where: {
      id: parseInt(annotation)
    }
  });

  if (!item) {
    res.send({
      code: 404,
      data: 'Annotation not found.'
    })
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
  })
});

export default router;
