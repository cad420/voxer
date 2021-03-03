import { ObjectID } from "mongodb";

type IGroupShared = {
  name: string;
  creator: ObjectID;
  createTime: number;
  applications: string[];
  datasets: ObjectID[];
  users: ObjectID[];
};

interface IGroupBackend extends IGroupShared {
  _id: ObjectID;
}

interface IGroupFrontEnd extends IGroupShared {
  id: string;
}

type DatasetGroup = {
  id: string;
  name: string;
  creator: string;
  createTime: number;
  datasets: Array<ObjectID>;
  users: Array<ObjectID>;
  applications: string[];
};

export default DatasetGroup;

export { IGroupShared, IGroupFrontEnd, IGroupBackend };
