export const DATASET_FILE = process.env.DATASET_FILE || "datasets.json";

export const PIPELINE_DIR = process.env.PIPELINE_DIR || process.cwd();

export const UPLOAD_PATH = process.env.UPLOAD_PATH || process.cwd();

export const PUBLIC_PATH = process.env.PUBLIC_PATH || process.cwd();

export const RENDER_SERVICE =
  process.env.RENDER_SERVICE || "ws://localhost:3000";
