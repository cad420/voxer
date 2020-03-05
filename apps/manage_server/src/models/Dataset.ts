import fs from "fs";
import config from "../config";

export type Dataset = {
  name: string;
  dimensions?: [number, number, number];
  variables: Array<{
    name: string;
    timesteps: number;
    path: string;
  }>;
};

class DatasetStore {
  filepath: string;
  datasets: Record<string, Dataset>;

  constructor() {
    this.filepath = config.datasets;
    this.load();
  }

  load() {
    if (!fs.existsSync(this.filepath)) {
      this.datasets = {};
    }

    try {
      const content = fs.readFileSync(this.filepath, "utf8");
      this.datasets = JSON.parse(content);
    } catch (err) {
      this.datasets = {};
    }
  }

  get(id: string) {
    return this.datasets[id];
  }

  getAll() {
    return this.datasets;
  }

  add(dataset: Dataset) {
    this.datasets[dataset.name] = dataset;
  }
}

export default new DatasetStore();
