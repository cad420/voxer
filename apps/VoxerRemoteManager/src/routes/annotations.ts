import express from "express";

const router = express.Router();

router.get("/:dataset/:axis/:index", (req, res) => {
  res.send({
    code: 200,
    data: '',
  });
});

router.post("/:dataset/:axis/:index", (req, res) => {
  const { name, variable, timestep } = req.params;
  const timestepValue = parseInt(timestep);

  const dataset = store.get(name);
  if (!dataset) {
    res.send({
      code: 404,
      data: "Not found"
    });
    return;
  }

  const variables = dataset.variables;
  const datasetVariable = variables.find(item => item.name === variable);
  if (!datasetVariable) {
    res.send({
      code: 404,
      data: "Not found"
    });
    return;
  }

  if (datasetVariable.timesteps < timestepValue) {
    res.send({
      code: 404,
      data: "Not found"
    });
    return;
  }

  res.send({
    code: 200,
    data: store.getHistogram(name, variable, timestepValue)
  });
});

export default router;
