import express from "express";
import mongodb from "mongodb";
import Dataset from "../models/Dataset";

type Axis = 'x' | 'y' | 'z';

function validate(axis: string, index: number): string {
  if (axis !== "z" && axis !== "y" && axis !== "x") {
    return "invalid axis";
  }

  if (isNaN(index) || index < 0) {
    return "invalid index";
  }

  return '';
}

const router = express.Router();

router.get("/:dataset/:axis/:index", async (req, res) => {
  const { dataset: id, axis } = req.params;
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

  const dataset: Dataset = await collection.findOne({ _id: id });

  if (!dataset) {
    res.send({
      code: 404,
      data: `dataset ${id} not found`,
    });
    return;
  }

  if (dataset.annotations[axis as Axis].length <= index) {
    res.send({
      code: 400,
      data: [],
    });
    return;
  }

  res.send({
    code: 200,
    data: dataset.annotations[axis as Axis][index].map(
      ({ tag, comment, type, coordinates }) => ({
        tag,
        comment,
        type,
        coordinates,
      })
    ),
  });
});

router.post("/:dataset/:axis/:index", async (req, res) => {
  const { dataset: id, axis } = req.params;
  const { tag, type, coordinates, comment = "" } = req.body;
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

  const dataset: Dataset = await collection.findOne({ _id: id });

  const annotation = {
    tag, type, coordinates, comment
  };

  await collection.updateOne({
    _id: dataset._id,
    [`annotations.${axis}`]: '1'
  }, {
    $push: { [index.toString()]:annotation }
  });

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
