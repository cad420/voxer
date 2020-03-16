import { PIPELINE_DIR } from "../config";
import fs from "fs-extra";
import path from "path";
import nanoid from "nanoid";
import { Pipeline, Scene } from "./voxer";

class PipelineStore {
  dir: string;
  pipelines: Record<string, Pipeline>;
  loading: Promise<void>;

  constructor() {
    this.dir = PIPELINE_DIR;
    this.pipelines = {};
    this.loading = this.load();
  }

  async load() {
    const files = await fs.readdir(this.dir);
    const tasks: Promise<void>[] = [];
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

  async add(params: Scene): Promise<string> {
    // TODO: validate
    const id = nanoid(12);
    const pipeline = { id, params } as Pipeline;
    this.pipelines[id] = pipeline;

    const filepath = path.join(this.dir, `${id}.json`);
    await fs.writeFile(filepath, JSON.stringify(pipeline, null, 2));

    return id;
  }

  async update(id: string, pipeline: Pipeline) {
    // TODO: validate
    this.pipelines[id] = pipeline;

    const filepath = path.join(this.dir, `${id}.json`);
    await fs.writeFile(filepath, JSON.stringify(pipeline, null, 2));
  }
}

export default new PipelineStore();
