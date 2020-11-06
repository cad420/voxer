import express from "express";
import multer from "multer";
import mongodb, { ObjectId } from "mongodb";
import { resolve } from "path";
import Dataset from "../models/Dataset";
import { RENDER_SERVICE, UPLOAD_PATH, REFINE_WATCH_DIR } from "../config";
import { getDatasetInfo } from "../rpc";
import childProcess from "child_process";
import fs from "fs-extra";
import Pipeline from "../models/Pipeline";

const { exec } = childProcess;

const router = express.Router();

const storage = multer.diskStorage({
  destination: (req, file, cb) => {
    cb(null, UPLOAD_PATH);
  },
  filename: (req, file, cb) => {
    cb(null, file.originalname);
  },
});
const upload = multer({ storage });

router.get("/", async (req, res) => {
  const database: mongodb.Db = req.app.get("database");
  const cache: Record<string, any> = req.app.get("datasets");

  const collection = database.collection("datasets");
  const datasets = await collection.find().toArray();

  res.send({
    code: 200,
    data: datasets.map((dataset: Dataset) => {
      const info = cache[dataset._id.toString()];
      return {
        id: dataset._id,
        name: dataset.name,
        dimensions: info ? info.dimensions : [],
      };
    }),
  });
});

router.post("/", upload.single("dataset"), async (req, res) => {
  const filename = req.file.filename;

  // TODO: make sure previous timestep is uploaded
  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("datasets");
  const result = await collection.insertOne({
    path: filename, // TODO
    name: filename,
  });
  const id = result.insertedId;

  const info = await getDatasetInfo(RENDER_SERVICE, {
    id,
    name: filename,
    path: resolve(UPLOAD_PATH, filename),
  });
  const cache = req.app.get("datasets");
  cache[id] = info;

  res.send({
    code: 200,
    data: id,
  });
});

router.get("/:id", async (req, res) => {
  const { id } = req.params;

  const database: mongodb.Db = req.app.get("database");
  const cache = req.app.get("datasets");

  const collection = database.collection("datasets");

  const dataset: Dataset = await collection.findOne({ _id: new ObjectId(id) });

  if (!dataset) {
    res.send({
      code: 404,
      data: "Not found",
    });
    return;
  }

  return res.send({
    code: 200,
    data: cache[id],
  });
});

async function createPipelineAndSave(filename: string, database: mongodb.Db) {
  let collection = database.collection("datasets");
  const dataset: Dataset = await collection.findOne({ path: filename });

  let id = '';
  if (dataset) {
    id = dataset._id.toHexString();
    console.log(`Refined data already exist: ${id}`);
  } else {
    const result = await collection.insertOne({
      path: filename,
      name: filename,
    });
    id = result.insertedId;
    console.log(`Refined data uploaded: ${id}`);
  }

  const info = await getDatasetInfo(RENDER_SERVICE, {
    id,
    name: filename,
    path: resolve(UPLOAD_PATH, filename),
  });

  collection = database.collection("pipelines");
  const exist: Pipeline = await collection.findOne({ "isosurfaces.0.dataset": id });

  if (exist) {
    console.log(`Refined data's pipeline already exist: ${exist._id.toHexString()}`);
    return info;
  }

  const maxSide = Math.max(info.dimensions[0], info.dimensions[1]);
  const pipeline: any = {
    type: 'single',
    comment: filename,
    tfcns: [],
    volumes: [],
    isosurfaces: [{ dataset: id, value: 100, color: "#aaaaaa", render: true }],
    camera: {
      width: 400,
      height: 400,
      pos: [0.0, 0.0, maxSide * 1.5],
      target: [0.0, 0.0, 0.0],
      up: [0.0, 1.0, 0.0],
      zoom: 1,
    },
  }
  const res = await collection.insertOne(pipeline);
  console.log(`Refined data's pipeline created: ${res.insertedId}`);
  
  return info;
}

function pollingRefineResult(watchDir: string, name: string, database: mongodb.Db, cache: any) {
  const finalFilename = `${name}_class001.mrc`;

  const intervalId = setInterval(async () => {
    try {
      const files = await fs.readdir(watchDir);
      const target = files.find(file => file === finalFilename);
      if (target) {
        console.log('Find refined data');
        const filepath = `${watchDir}/${finalFilename}`;
        const destFilepath = `${UPLOAD_PATH}/${finalFilename}`;
        
        clearInterval(intervalId);
        
        await fs.copyFile(filepath, destFilepath);
        const info = await createPipelineAndSave(finalFilename, database);
        cache[info.id] = info;
        console.log('Refined data Processed: ' + info.id);
      }
    } catch (err) {
      console.error(err.message);
      clearInterval(intervalId);
    }
  }, 10000);
}

const script = `${process.cwd()}/refine.sh`;
console.log(`Refine Script: ${script}`)

router.post("/refine/:name", async (req, res) => {
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

    console.log(`Task submited: ` + stdout.substring(0, 100));
    const database: mongodb.Db = req.app.get("database");
    const cache = req.app.get("datasets");
    console.log(`Refine watch directory: ${REFINE_WATCH_DIR}`);
    pollingRefineResult(REFINE_WATCH_DIR, name, database, cache);

    return res.send({
      code: 200,
    });
  });
});

export default router;
