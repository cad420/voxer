import fs from "fs";
import utils from "util";
import { resolve } from "path";
import { pipeline } from "stream";
import { ObjectID } from "mongodb";
import { FastifyInstance } from "fastify";
import { options } from "../index";
import Dataset from "../models/Dataset";
import DatasetGroup from "../models/DatasetGroup";

export const pump = utils.promisify(pipeline);

async function routes(server: FastifyInstance) {
  const db = server.mongo.db;
  if (!db) {
    return;
  }

  /**
   * get all datasets
   */
  server.get<{
    Querystring: {
      page: string;
      size: string;
    };
  }>("/datasets", async (req) => {
    const datasets = db.collection("datasets");

    let page = parseInt(req.query.page);
    if (isNaN(page) || page <= 0) {
      page = 1;
    }

    let size = parseInt(req.query.size);
    if (isNaN(page) || size <= 0) {
      size = 10;
    }

    const total = await datasets.countDocuments();
    const list = await datasets
      .find(
        {},
        {
          projection: {
            _id: false,
            id: {
              $toString: "$_id",
            },
            name: true,
            dimensions: true,
            range: true,
          },
        }
      )
      .skip((page - 1) * size)
      .limit(size)
      .toArray();

    return {
      code: 200,
      data: {
        list,
        total,
      },
    };
  });

  /**
   * add a dataset by upload
   */
  server.post<{
    Body:
      | {
          name: string;
          dataset: Buffer;
          path?: string;
        }
      | undefined;
  }>("/datasets", async (req, res) => {
    let name = "";
    let path = "";
    if (req.body) {
      path = req.body.path;
      name = req.body.name;
    } else {
      const data = await req.file();
      path = data.filename;
      name = (data.fields.name as any).value;
      const filepath = resolve(options.storage, path);
      await pump(data.file, fs.createWriteStream(filepath));
    }

    const collection = db.collection("datasets");

    const exist = (await collection.findOne(
      { path },
      {
        projection: {
          id: {
            $toString: "$_id",
          },
          histogram: true,
        },
      }
    )) as Dataset;
    let id = "";
    if (exist) {
      id = exist.id;
      if (exist.histogram && exist.histogram.length > 0) {
        res.send({
          code: 200,
          data: id,
        });
        return;
      }
    } else {
      const result = await collection.insertOne({
        path, // TODO
        name,
        dimensions: [1, 1, 1],
        histogram: [],
        range: [1, 1],
        groups: [],
      });
      id = result.insertedId;
    }

    try {
      const worker = server.getWorker();
      const info = await worker.getDatasetInfo(id, name, path);
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

      return {
        code: 200,
        data: id,
      };
    } catch (err) {
      await collection.deleteOne({ _id: new ObjectID(id) });
      return {
        code: 400,
        data: err.message,
      };
    }
  });

  /**
   * get a dataset info
   */
  server.get<{
    Params: { id: string };
  }>("/datasets/:id", async (req, res) => {
    const { id } = req.params;

    const collection = db.collection<Dataset>("datasets");

    const dataset = await collection.findOne<Dataset>(
      { _id: new ObjectID(id) },
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
    );

    if (dataset.histogram.length === 0) {
      try {
        const worker = server.getWorker();
        const info = await worker.getDatasetInfo(
          dataset.id,
          dataset.name,
          dataset.path
        );
        await collection.updateOne(
          { _id: new ObjectID(dataset.id) },
          {
            $set: {
              dimensions: info.dimensions,
              histogram: info.histogram,
              range: info.range,
            },
          }
        );
      } catch (err) {
        res.send({
          code: 500,
          data: err.message,
        });
        return;
      }
    }

    if (!dataset) {
      res.send({
        code: 404,
        data: "Not found",
      });
      return;
    }

    return res.send({
      code: 200,
      data: dataset,
    });
  });

  /**
   * delete a dataset
   */
  server.delete<{
    Params: { id: string };
  }>("/datasets/:id", async (req, res) => {
    const { id } = req.params;
    const datasetId = new ObjectID(id);
    const datasets = db.collection<Dataset>("datasets");
    const result = await datasets.findOneAndDelete({
      _id: new ObjectID(id),
    });
    if (!result.value) {
      res.send({
        code: 404,
        data: "Dataset not found",
      });
      return;
    }
    if (!result.ok) {
      res.send({
        code: 500,
        data: "Failed to delete dataset",
      });
      return;
    }

    const dataset = result.value;
    const groups = db.collection<DatasetGroup>("datasets");
    const op = await groups.updateMany(
      { _id: { $in: dataset.groups || [] } },
      {
        $pull: {
          datasets: datasetId,
        },
      }
    );

    if (!op.result.ok) {
      res.send({
        code: 500,
        data: "Failed to delete dataset",
      });
      return;
    }

    res.send({ code: 200 });
  });
}

export default routes;
