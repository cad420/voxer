import fs from "fs-extra";
import { DATASET_FILE } from "../config";
import { Dataset } from "./voxer";
import DatasetMessager from "./DatasetMessager";

class DatasetStore {
  filepath: string;
  datasets: Record<string, Dataset>;
  messager: DatasetMessager;
  histograms: Record<string, Record<string, Array<Array<number>>>>;

  constructor() {
    this.filepath = DATASET_FILE;
    this.messager = new DatasetMessager();
    this.histograms = {};
    this.messager.ws.onmessage = event => {
      const data = event.data as string;
      const info = JSON.parse(data);
      const dataset = this.datasets[info.name];
      if (!dataset) return;

      const variable = dataset.variables.find(
        item => item.name === info.variable
      );
      if (!variable) return;

      this.histograms[info.name][info.variable][info.timestep] = info.histogram;
    };
    this.load();
  }

  load() {
    if (!fs.existsSync(this.filepath)) {
      this.datasets = {};
    }

    try {
      const content = fs.readFileSync(this.filepath, "utf8");
      this.datasets = JSON.parse(content);

      Object.values(this.datasets).forEach(dataset => {
        dataset.variables.forEach(variable => {
          const datasetHistograms = this.histograms[dataset.name];
          if (!datasetHistograms) {
            this.histograms[dataset.name] = {};
          }
          this.histograms[dataset.name][variable.name] = [];
        });
      });
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

  getHistogram(name: string, variable: string, timestep: number) {
    return this.histograms[name][variable][timestep];
  }

  add(dataset: Dataset) {
    this.datasets[dataset.name] = dataset;
    dataset.variables.forEach(variable => {
      const datasetHistograms = this.histograms[dataset.name];
      if (!datasetHistograms) {
        this.histograms[dataset.name] = {};
      }
      this.histograms[dataset.name][variable.name] = [];
    });
    this.messager.post(dataset);
    this.save();
  }
}

export default new DatasetStore();
