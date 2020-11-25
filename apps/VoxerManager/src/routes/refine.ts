import express from "express";
import mongodb, { ObjectID } from "mongodb";
import { resolve } from "path";
import Dataset from "../models/Dataset";
import { RENDER_SERVICE, UPLOAD_PATH, REFINE_WATCH_DIR } from "../config";
import { getDatasetInfo } from "../rpc";
import childProcess from "child_process";
import fs from "fs-extra";
import Pipeline from "../models/Pipeline";

const router = express.Router();

const { exec } = childProcess;

async function createPipelineAndSave(
  filename: string,
  database: mongodb.Db
): Promise<string> {
  let collection = database.collection("datasets");
  const dataset: Dataset = await collection.findOne({ path: filename });

  let id = "";
  let maxDim = 300;
  if (dataset) {
    id = dataset._id.toHexString();
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
    id = result.insertedId;

    const info = await getDatasetInfo(RENDER_SERVICE, {
      id,
      name: filename,
      path: resolve(UPLOAD_PATH, filename),
    });

    if (info.error) {
      await collection.deleteOne({ _id: new ObjectID(id) });
      throw new Error(info.error);
    }

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
  }

  collection = database.collection("pipelines");
  const exist: Pipeline = await collection.findOne({
    "isosurfaces.0.dataset": id,
  });

  if (exist) {
    console.log(
      `Refined data's pipeline already exist: ${exist._id.toHexString()}`
    );
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
  string,
  {
    id: number;
    finished: boolean;
    data: string[];
  }
> = {};

function pollingRefineResult(
  watchDir: string,
  name: string,
  database: mongodb.Db
) {
  const tmpResultReg = new RegExp(name + "_it(\\S*)_half1_class001.mrc");
  const finalFilename = `${name}_class001.mrc`;
  const targetSize = 360 * 360 * 360 * 4 + 1024;

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

        if (RefinedData[name]) {
          RefinedData[name].finished = true;
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

          if (RefinedData[name]) {
            RefinedData[name].data.push(filename);
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
    data: Object.entries(RefinedData).map(([name, info]) => {
      return {
        name,
        ...info,
      };
    }),
  });
});

router.post("/:name", async (req, res) => {
  const { name } = req.params;
  if (!name) {
    res.send({
      code: 400,
      data: "Should have refine name",
    });
    return;
  }

  exec(`sh ${script}`, (err, stdout, stderr) => {
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
        data: stdout.substring(0, 100),
      });
    }

    const id = parseInt(matched[1]);
    RefinedData[name] = {
      id,
      finished: false,
      data: [],
    };
    console.log(`Task submited: ${id}`);

    const database: mongodb.Db = req.app.get("database");
    console.log(`Refine watch directory: ${REFINE_WATCH_DIR}`);
    pollingRefineResult(REFINE_WATCH_DIR, name, database);

    return res.send({
      code: 200,
    });
  });
});

export default router;
