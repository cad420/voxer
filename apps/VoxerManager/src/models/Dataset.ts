type Dataset = {
  id: string;
  name: string;
  path: string;
  dimensions: [number, number, number];
  histogram: number[];
  range: [number, number];
};

export default Dataset;
