import typescript from "@rollup/plugin-typescript";
import commonjs from "@rollup/plugin-commonjs";
import replace from "@rollup/plugin-replace";
import resolve from "@rollup/plugin-node-resolve";
import json from "@rollup/plugin-json";

const mode = process.env.NODE_ENV;

const plugins = [
  replace({
    "process.env.browser": true,
    "process.env.NODE_ENV": mode,
  }),
  typescript(),
  json(),
  resolve({
    browser: true,
  }),
  commonjs(),
];

export default [
  {
    input: "src/extension.ts",
    output: {
      file: "jupyter-widget-voxer/static/extension.js",
      format: "amd",
    },
    plugins,
    external: ["@jupyter-widgets/base"],
  },
  {
    input: "src/notebook.ts",
    output: [
      {
        file: "jupyter-widget-voxer/static/index.js",
        format: "amd",
      },
    ],
    plugins,
    external: ["@jupyter-widgets/base"],
  },
  {
    input: "src/embed.ts",
    output: [
      {
        file: "dist/index.js",
        format: "amd",
      },
    ],
    plugins,
    external: ["@jupyter-widgets/base"],
  },
  {
    input: "src/jupyterlab-plugin.ts",
    output: [
      {
        file: "dist/jupyterlab-plugin.js",
        format: "amd",
      },
    ],
    plugins,
    external: ["@jupyter-widgets/base"],
  },
];
