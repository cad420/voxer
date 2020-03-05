import config from "../config";
import fs from "fs-extra";
import path from "path";
import nanoid from "nanoid";

export namespace voxer {
  export type Dataset = {};

  export type Volume = {
    dataset: number;
    tfcn: number;
    spacing: [number, number, number];
    render: boolean;
  };

  export type Isosurface = {
    volume: number;
    isovalue: number;
    render: boolean;
  };

  export type Camera = {
    position: [number, number, number];
    target: [number, number, number];
    up: [number, number, number];
    width: number;
    height: number;
  };

  export type TfcnControlPoint = {
    x: number;
    y: number;
    color: number;
  };

  export type TransferFunction = Array<TfcnControlPoint>;

  export type Pipeline = {
    id: string;
    params: {
      datasets: Dataset;
      tfcns: Array<TransferFunction>;
      volumes: Array<Volume>;
      isosurfaces: Array<Isosurface>;
      camera: Camera;
    };
  };
}

export type Pipeline = voxer.Pipeline;

class PipelineStore {
  dir: string;
  pipelines: Record<string, Pipeline>;
  loading: Promise<any>;

  constructor() {
    this.dir = config.pipelines;
    this.pipelines = {};
    this.loading = this.load();
  }

  async load() {
    const files = await fs.readdir(this.dir);
    const tasks: Promise<any>[] = [];
    files.forEach(file => {
      if (!file.endsWith(".json") || file === "datasets.json") return;

      const filepath = path.join(this.dir, file);
      const task = fs.readFile(filepath, "utf8").then(content => {
        const pipeline = JSON.parse(content);
        this.pipelines[pipeline.id] = pipeline;
      });
      tasks.push(task);
    });
    await Promise.all(tasks);
  }

  async get(id: string) {
    await this.loading;

    return this.pipelines[id];
  }

  async getAll() {
    await this.loading;

    return this.pipelines;
  }

  async add(pipeline: any): Promise<string> {
    // TODO: validate
    const id = nanoid(12);
    this.pipelines[id] = pipeline as Pipeline;

    const filepath = path.join(this.dir, `${id}.json`);
    await fs.writeFile(filepath, JSON.stringify(pipeline, null, 2));

    return id;
  }

  update(id: string, pipeline: Pipeline) {
    // TODO: validate
    this.pipelines[id] = pipeline;
  }
}

export default new PipelineStore();
