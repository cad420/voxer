import express from "express";
import mongodb, { ObjectID } from "mongodb";
import { resolve } from "path";
import Dataset from "../models/Dataset";
import { UPLOAD_PATH, REFINE_BASE_DIR } from "../config";
import { getDatasetInfo } from "../worker_api/jsonrpc";
import childProcess from "child_process";
import fs from "fs-extra";

const router = express.Router();

const { exec } = childProcess;

async function createPipelineAndSave(
  filename: string,
  database: mongodb.Db
): Promise<string> {
  let collection = database.collection("datasets");
  const dataset = (await collection.findOne(
    { path: filename },
    {
      projection: {
        _id: false,
        id: {
          $toString: "$_id",
        },
        name: true,
        path: true,
        dimensions: true,
        histogram: true,
        range: true,
      },
    }
  )) as Dataset;

  let id = "";
  let maxDim = 300;
  if (dataset) {
    id = dataset.id;
    maxDim = Math.max(dataset.dimensions[0], dataset.dimensions[1]);
    console.log(`Refined data already exist: ${id}`);
  } else {
    const result = await collection.insertOne({
      path: filename,
      name: filename,
      dimensions: [1, 1, 1],
      histogram: [],
      range: [1, 1],
    });
    id = result.insertedId.toHexString();

    try {
      const info = await getDatasetInfo(id, filename, filename);
      await collection.updateOne(
        { _id: new ObjectID(id) },
        {
          $set: {
            dimensions: info.dimensions,
            histogram: info.histogram,
            range: info.range,
          },
        }
      );
      maxDim = Math.max(info.dimensions[0], info.dimensions[1]);
      console.log(`Refined data uploaded: ${id}`);
    } catch (err) {
      await collection.deleteOne({ _id: new ObjectID(id) });
      throw new Error(err.message);
    }
  }

  collection = database.collection("pipelines");
  const exist = await collection.findOne(
    {
      "isosurfaces.0.dataset": id,
    },
    {
      projection: {
        _id: true,
      },
    }
  );

  if (exist) {
    console.log(`Refined data's pipeline already exist: ${exist._id}`);
    return id;
  }

  const pipeline: any = {
    type: "single",
    comment: filename,
    tfcns: [],
    volumes: [],
    isosurfaces: [{ dataset: id, value: 100, color: "#aaaaaa", render: true }],
    camera: {
      width: 400,
      height: 400,
      pos: [0.0, 0.0, maxDim * 1.5],
      target: [0.0, 0.0, 0.0],
      up: [0.0, 1.0, 0.0],
      zoom: 1,
    },
  };
  const res = await collection.insertOne(pipeline);
  console.log(`Refined data's pipeline created: ${res.insertedId}`);

  return id;
}

const RefinedData: Record<
  number,
  {
    id: number;
    name: string;
    finished: boolean;
    data: string[];
  }
> = {};

async function deleteExsitDatasetAndPipeline(
  tmpReg: RegExp,
  finalName: string,
  database: mongodb.Db
) {
  let collection = database.collection("datasets");
  const datasets = await collection
    .find(
      {},
      {
        projection: {
          id: {
            $toString: "$_id",
          },
          name: true,
        },
      }
    )
    .toArray();

  const targets = datasets
    .filter((dataset: any) => {
      return tmpReg.test(dataset.name) || dataset.name === finalName;
    })
    .map((dataset) => dataset.id);

  const datasetsDeleted = await collection.deleteMany({
    _id: { $in: targets.map((id) => new ObjectID(id as string)) },
  });
  console.log("Old refine data deleted: " + datasetsDeleted.deletedCount);

  collection = database.collection("pipelines");
  const pipelinesDeleted = await collection.deleteMany({
    "isosurfaces.0.dataset": { $in: targets },
  });
  console.log("Old refine pipeline deleted: " + pipelinesDeleted.deletedCount);
}

async function pollingRefineResult(
  taskId: number,
  watchDir: string,
  name: string,
  database: mongodb.Db,
  size: number
) {
  const tmpResultReg = new RegExp(name + "_it(\\S*)_half1_class001.mrc");
  const finalFilename = `${name}_class001.mrc`;
  const targetSize = size * size * size * 4 + 1024;

  await deleteExsitDatasetAndPipeline(tmpResultReg, finalFilename, database);

  let lastIteration = 0;
  const intervalId = setInterval(async () => {
    try {
      const files = await fs.readdir(watchDir);
      const target = files.find((file) => file === finalFilename);
      if (target) {
        const filepath = resolve(watchDir, finalFilename);
        const stat = await fs.stat(filepath);
        if (stat.size !== targetSize) {
          console.log(
            `Find refined data: ${filepath}, but size is not correct: ${stat.size}`
          );
          return;
        }

        console.log("Find refined data");
        const destFilepath = resolve(UPLOAD_PATH, finalFilename);

        if (RefinedData[taskId]) {
          RefinedData[taskId].finished = true;
        }

        await fs.copyFile(filepath, destFilepath);
        const id = await createPipelineAndSave(finalFilename, database);

        console.log("Refined data Processed: " + id);
        clearInterval(intervalId);
      } else {
        let filename = "";
        let maxIteration = lastIteration;
        files.forEach((file) => {
          const res = file.match(tmpResultReg);
          if (res) {
            const iter = parseInt(res[1]);
            if (!isNaN(iter) && iter > maxIteration) {
              filename = file;
              maxIteration = iter;
            }
          }
        });

        if (filename) {
          const filepath = resolve(watchDir, filename);
          const stat = await fs.stat(filepath);
          if (stat.size !== targetSize) {
            console.log(
              `Find refined data: ${filepath}, but size is not correct: ${stat.size}`
            );
            return;
          }

          console.log("Find refined data");
          const destFilepath = resolve(UPLOAD_PATH, filename);
          await fs.copyFile(filepath, destFilepath);
          const id = await createPipelineAndSave(filename, database);

          if (RefinedData[taskId]) {
            RefinedData[taskId].data.push(filename);
          }

          lastIteration = maxIteration;
          console.log(
            `Refined data Processed, iteration: ${maxIteration}, id: ${id}`
          );
        }
      }
    } catch (err) {
      console.error(err.message);
      clearInterval(intervalId);
    }
  }, 10000);
}

const script = resolve(process.cwd(), "refine.sh");
console.log(`Refine Script: ${script}`);

router.get("/info", async (req, res) => {
  res.send({
    code: 200,
    data: Object.values(RefinedData),
  });
});

router.post("/", async (req, res) => {
  const { nodes, process, path, output, input, init, size } = req.body;

  exec(
    `sh ${script} ${nodes} ${process} ${path} ${output} ${input} ${init} ${size}`,
    async (err, stdout, stderr) => {
      if (err) {
        return res.send({
          code: 400,
          data: err.message,
        });
      } else if (stderr) {
        return res.send({
          code: 400,
          data: stderr,
        });
      }

      const jobIdRegExp = new RegExp(/Submitted batch job (\d+)/);
      const matched = stdout.match(jobIdRegExp);
      if (!matched || !matched[1]) {
        return res.send({
          code: 400,
          data: stdout.substring(0, 500),
        });
      }

      const dataDim = parseInt(size);
      if (isNaN(dataDim) || dataDim <= 0) {
        return res.send({
          code: 400,
          data: "invalid size",
        });
      }

      const lastSlash = ((output as string) || "").lastIndexOf("/");
      const prefix = ((output as string) || "").substring(0, lastSlash);
      const name = ((output as string) || "").substring(lastSlash + 1);
      if (!name) {
        return res.send({
          code: 400,
          data: "Invalid output: " + output,
        });
      }

      const taskId = parseInt(matched[1]);
      console.log(`Task submited: ${taskId}`);

      try {
        const database: mongodb.Db = req.app.get("database");
        const watchDir = resolve(REFINE_BASE_DIR, path, prefix);

        await pollingRefineResult(taskId, watchDir, name, database, dataDim);

        console.log(`Refine watch directory: ${watchDir}`);
        RefinedData[taskId] = {
          id: taskId,
          name,
          finished: false,
          data: [],
        };

        return res.send({
          code: 200,
        });
      } catch (err) {
        return res.send({
          code: 400,
          data: err.message,
        });
      }
    }
  );
});

export default router;
