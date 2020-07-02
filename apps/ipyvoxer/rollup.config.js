import typescript from "@rollup/plugin-typescript";
import commonjs from "@rollup/plugin-commonjs";
import replace from "@rollup/plugin-replace";
import resolve from "@rollup/plugin-node-resolve";
import json from "@rollup/plugin-json";
import styles from "rollup-plugin-styles";

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
  styles(),
];

export default [
  {
    input: "src/extension.ts",
    output: {
      file: "ipyvoxer/static/extension.js",
      format: "amd",
      sourcemap: true,
    },
    plugins,
    external: ["@jupyter-widgets/base"],
    cache: true,
  },
  {
    input: "src/notebook.ts",
    output: [
      {
        file: "ipyvoxer/static/index.js",
        format: "amd",
        sourcemap: true,
      },
    ],
    plugins,
    external: ["@jupyter-widgets/base"],
    cache: true,
  },
  {
    input: "src/embed.ts",
    output: [
      {
        file: "dist/index.js",
        format: "amd",
        sourcemap: true,
      },
    ],
    plugins,
    external: ["@jupyter-widgets/base"],
    cache: true,
  },
  {
    input: "src/jupyterlab-plugin.ts",
    output: [
      {
        file: "dist/jupyterlab-plugin.js",
        format: "amd",
        sourcemap: true,
      },
    ],
    plugins,
    external: ["@jupyter-widgets/base"],
    cache: true,
  },
];
