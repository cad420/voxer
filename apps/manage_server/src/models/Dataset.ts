import fs from "fs-extra";
import config from "../config";
import { Dataset } from "./voxer";

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

  async save() {
    await fs.writeFile(this.filepath, JSON.stringify(this.datasets, null, 2));
  }

  get(id: string) {
    return this.datasets[id];
  }

  getAll() {
    return this.datasets;
  }

  add(dataset: Dataset) {
    this.datasets[dataset.name] = dataset;
    this.save();
  }
}

export default new DatasetStore();
