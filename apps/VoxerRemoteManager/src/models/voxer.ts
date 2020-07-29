type Histogram = Array<number>;
export type Dataset = {
  name: string;
  dimensions?: [number, number, number];
  variables: Array<{
    name: string;
    timesteps: number;
    path: string;
  }>;
};

export type Volume = {
  dataset: number;
  tfcn: number;
  spacing: [number, number, number];
  render: boolean;
};

export type Isosurface = {
  color: string;
  dataset: number;
  isovalue: number;
  render: boolean;
};

export type Camera = {
  position: [number, number, number];
  target: [number, number, number];
  up: [number, number, number];
  width: number;
  height: number;
};

export type TfcnControlPoint = {
  x: number;
  y: number;
  color: number;
};

export type Scene = {
  datasets: Dataset;
  tfcns: Array<TransferFunction>;
  volumes: Array<Volume>;
  isosurfaces: Array<Isosurface>;
  camera: Camera;
};

export type TransferFunction = Array<TfcnControlPoint>;

export type Pipeline = {
  id: string;
  params: Scene;
};
