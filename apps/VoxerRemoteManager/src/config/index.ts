export const UPLOAD_PATH = process.env.UPLOAD_PATH || process.cwd();

export const PUBLIC_PATH = process.env.PUBLIC_PATH || process.cwd();

export const DATABASE = process.env.DATABASE || '127.0.0.1:27017'

export const RENDER_SERVICE =
  process.env.RENDER_SERVICE || "ws://localhost:3040";
