import fs from "fs-extra";
import { DATASET_FILE } from "../config";
import { Dataset } from "./voxer";
import DatasetMessager from "./DatasetMessager";

class DatasetStore {
  filepath: string;
  datasets: Record<string, Dataset>;
  messager: DatasetMessager;

  constructor() {
    this.filepath = DATASET_FILE;
    this.messager = new DatasetMessager();
    this.load();
  }

  load() {
    if (!fs.existsSync(this.filepath)) {
      this.datasets = {};
    }

    try {
      const content = fs.readFileSync(this.filepath, "utf8");
      this.datasets = JSON.parse(content);
      this.messager.post(Object.values(this.datasets));
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
    this.messager.post(dataset);
    this.save();
  }
}

export default new DatasetStore();
