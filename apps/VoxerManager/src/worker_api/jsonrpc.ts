import jayson from "jayson";
import Annotation from "../models/Annotation";

function parseWorkerAddress(address: string): { host: string; port: number } {
  let host = address;
  let port = 80;

  const commaIdx = address.lastIndexOf(":");
  if (commaIdx != -1) {
    const value = parseInt(address.substring(commaIdx + 1));
    if (!isNaN(value) && value > 0 && value < 65536) {
      port = value;
    }

    host = address.substring(0, commaIdx);
  }

  return { host, port };
}

const worker = parseWorkerAddress("");

const client = jayson.Client.http({
  host: worker.host,
  port: worker.port,
  path: "/jsonrpc",
});

function RPCCall<
  ParamsType extends Array<any> | Record<string, any> | undefined,
  ResultType
>(name: string, params: ParamsType): Promise<ResultType> {
  return new Promise((resolve, reject) => {
    client.request(
      name,
      params,
      (err: Error, response: jayson.JSONRPCResultLike) => {
        if (err) {
          reject(err);
          return;
        }

        if (response.error) {
          reject(response.error);
          return;
        }

        resolve(response.result);
      }
    );
  });
}

export async function add(a: number, b: number): Promise<number> {
  return RPCCall("add", [a, b]);
}

export async function getDatasetInfo(
  id: string,
  name: string,
  path: string
): Promise<{
  id: string;
  dimensions: [number, number, number];
  histogram: number[];
  range: [number, number];
}> {
  return RPCCall("get_dataset_info", [id, name, path]);
}

export async function applyLevelSet(
  dataset: string,
  axis: string,
  index: number,
  annotations: Annotation[]
): Promise<Annotation[]> {
  return RPCCall("apply_levelset", { annotations, dataset, axis, index });
}

export async function applyGrabCut(
  dataset: string,
  axis: string,
  index: number,
  annotations: Annotation[]
): Promise<Annotation[]> {
  return RPCCall("apply_grabcut", [annotations, dataset, axis, index]);
}
