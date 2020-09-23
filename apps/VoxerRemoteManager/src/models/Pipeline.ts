import { ObjectID } from "mongodb";

interface Pipeline {
  _id: ObjectID;
  comment: string;
  type?: string;
  tfcns: Array<Array<{ x: number; y: number; color: string }>>;
  type?: string;
  volumes: Array<{
    dataset: ObjectID;
    tfcn: number;
    spacing: [number, number, number];
    render: boolean;
  }>;
  isosurfaces: Array<{
    dataset: ObjectID;
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
