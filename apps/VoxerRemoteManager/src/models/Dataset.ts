import mongodb, { ObjectID } from "mongodb";
import { resolve } from "path";
import { UPLOAD_PATH } from "../config";
import { RENDER_SERVICE } from "../config";
import { getDatasetInfo } from "../rpc";

type Dataset = {
  _id: ObjectID;
  name: string;
  path: string;
  dimensions: [number, number, number];
  histogram: number[];
  range: [number,number];
};

export async function getExistDatasetInfo(database: mongodb.Db) {
  const collection = database.collection("datasets");
  const datasets = await collection.find().toArray();

  const tasks = datasets.map(async (item) => {
    if (item.dimensions && item.dimensions[0] !== 1) {
      return;
    }

    const id = item._id.toHexString();
    const res = await getDatasetInfo(RENDER_SERVICE, {
      id,
      name: item.name,
      path: resolve(UPLOAD_PATH, item.path),
    });
    await collection.updateOne({ _id: item._id }, {
      $set: {
        dimensions: res.dimensions,
        histogram: res.histogram,
        range: res.range
      }
    });
  });

  return Promise.all(tasks);
}

export default Dataset;
