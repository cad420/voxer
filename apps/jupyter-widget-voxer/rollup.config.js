import typescript from "@rollup/plugin-typescript";
import commonjs from "@rollup/plugin-commonjs";
import replace from "@rollup/plugin-replace";
import resolve from "@rollup/plugin-node-resolve";
import json from "@rollup/plugin-json";

const mode = process.env.NODE_ENV;

export default [
  {
    input: "src/extension.ts",
    output: {
      file: "jupyter-widget-voxer/nbextension/static/index.js",
      format: "amd",
    },
    plugins: [
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
    ],
    external: ["@jupyter-widgets/base"],
  },
  {
    input: "src/index.ts",
    output: [
      {
        file: "dist/index.js",
        format: "amd",
      },
    ],
    plugins: [
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
    ],
    external: ["@jupyter-widgets/base"],
  },
];
