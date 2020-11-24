interface Pipeline {
  id: string;
  comment: string;
  tfcns: Array<Array<{ x: number; y: number; color: string }>>;
  type?: string;
  volumes: Array<{
    dataset: string;
    tfcn: number;
    spacing: [number, number, number];
    render: boolean;
  }>;
  isosurfaces: Array<{
    dataset: string;
    value: number;
    color: string;
    render: boolean;
  }>;
  camera: {
    width: number;
    height: number;
    type: string;
    position: [number, number, number];
    target: [number, number, number];
    up: [number, number, number];
  };
}

export default Pipeline;
