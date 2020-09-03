import { ObjectID } from 'mongodb';
import Annotation from './Annotation';

type Dataset = {
  _id: ObjectID;
  name: string;
  path: string;
  tags?: Array<{
    name: string;
    color: string;
  }>;
  annotations?: {
    z: Array<Array<Annotation>>; // 使用map, tag为id?
    y: Array<Array<Annotation>>;
    x: Array<Array<Annotation>>;
  };
};

export default Dataset;