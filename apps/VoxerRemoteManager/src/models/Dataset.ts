import { ObjectID } from 'mongodb';

type Dataset = {
  _id: ObjectID;
  name: string;
  path: string;
};

export default Dataset;