import path from "path";

export default {
  datasets:
    process.env.DATASETS_FILE ||
    path.resolve(__dirname, "../../public/datasets.json"),
  pipelines:
    process.env.PIPELINES_DIR || path.resolve(__dirname, "../../public/")
};
