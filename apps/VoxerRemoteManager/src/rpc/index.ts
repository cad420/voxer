import Annotation from "../models/Annotation";
import fetch from "node-fetch";

function request(server: string, uri: string, options: any = {}) {
  return fetch(`http://${server}${uri}`, {
    ...options,
    headers: {
      ...(options.headers || {}),
    },
  }).then((res) => res.json());
}

export function get(server: string, uri: string, params?: any) {
  if (!params) {
    return request(server, uri);
  }

  const query = Object.keys(params)
    .filter((key) => params[key] !== undefined)
    .map(
      (key) => `${encodeURIComponent(key)}=${encodeURIComponent(params[key])}`
    )
    .join("&");

  return request(server, query.length > 0 ? `${uri}?${query}` : uri);
}

export function post(server: string, uri: string, data: any) {
  return request(server, uri, {
    method: "post",
    body: JSON.stringify(data),
    headers: {
      "Content-Type": "application/json;charset=UTF-8",
    },
  });
}

export function applyLevelSet(
  server: string,
  dataset: string,
  axis: string,
  index: number,
  annotations: Annotation[]
): Promise<Annotation[]> {
  return post(server, "/annotations", {
    function: "apply_level_set",
    params: {
      dataset,
      axis,
      index,
      annotations,
    },
  });
}

export function getDatasetInfo(
  server: string,
  dataset: {
    id: string;
    name: string;
    path: string;
  }
): Promise<{
  id: string;
  dimensions: [number, number, number];
  histogram: number[];
  range: [number, number];
  error?: string;
}> {
  return post(server, "/datasets", {
    function: "query_dataset",
    params: dataset,
  });
}
