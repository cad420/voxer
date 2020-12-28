import nanoid from "nanoid";
import { DatasetInfo } from "../models/Dataset";

const msgpack = require("@ygoe/msgpack");

export interface RPCRequest {
  caller: string;
  id: string;
  method: string;
  params: Array<any>;
}

export interface RPCResponse {
  caller: string;
  id: string;
  method: string;
  result?: any;
  error?: {
    code: number;
    messsage: string;
  };
}

type RPCCallHandler = {
  resolve: (data?: any) => void;
  reject: (reason?: any) => void;
};

class WorkerAPICaller {
  rpc: Map<string, RPCCallHandler>;
  send: (data: RPCRequest) => void;

  constructor() {
    this.rpc = new Map();
  }

  responsed = (msg: Buffer) => {
    const response: RPCResponse = msgpack.deserialize(msg);
    const id = response.id;

    if (!id || !this.rpc.has(id)) {
      return;
    }

    const handler = this.rpc.get(id);
    if (response.error) {
      handler.reject(response.error);
    } else {
      handler.resolve(response.result);
    }
  };

  getDatasetInfo = (
    id: string,
    name: string,
    path: string
  ): Promise<DatasetInfo> => {
    const data = {
      caller: "VoxerManager",
      id: nanoid(10),
      method: "get_dataset_info",
      params: [id, name, path],
    };

    this.send(data);

    const promise = new Promise<DatasetInfo>((resolve, reject) => {
      this.rpc.set(data.id, {
        resolve,
        reject,
      });
    });

    return promise;
  };
}

export default WorkerAPICaller;
