export const UPLOAD_PATH = process.env.UPLOAD_PATH || process.cwd();

export const PUBLIC_PATH = process.env.PUBLIC_PATH || process.cwd();

export const DATABASE_PATH = process.env.DATABASE_PATH || 'data.sqlite'

export const RENDER_SERVICE =
  process.env.RENDER_SERVICE || "ws://localhost:3000";
