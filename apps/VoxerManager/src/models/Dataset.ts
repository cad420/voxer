import mongodb, { ObjectID } from "mongodb";
import { getDatasetInfo } from "../worker_api/jsonrpc";

type Dataset = {
  id: string;
  name: string;
  path: string;
  dimensions: [number, number, number];
  histogram: number[];
  range: [number, number];
};

export default Dataset;
