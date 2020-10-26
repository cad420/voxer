import mongodb, { ObjectID } from "mongodb";
import { resolve } from "path";
import { UPLOAD_PATH } from "../config";
import { RENDER_SERVICE } from "../config";
import { getDatasetInfo } from "../rpc";

type Dataset = {
  _id: ObjectID;
  name: string;
  path: string;
};

export async function getExistDatasetInfo(
  database: mongodb.Db,
  cache: Record<string, any>
) {
  const collection = database.collection("datasets");
  const datasets = await collection.find().toArray();

  datasets.forEach(async (item) => {
    const id = item._id.toHexString();
    const res = await getDatasetInfo(RENDER_SERVICE, {
      id,
      name: item.name,
      path: resolve(UPLOAD_PATH, item.path),
    });
    cache[id] = res;
  });
}

export default Dataset;
