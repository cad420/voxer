import { ObjectID } from "mongodb";

interface DatasetGroup {
  _id: ObjectID;
  name: string;
  variables: Array<{
    name: string;
    timesteps: Array<ObjectID>;
  }>;
}

export default DatasetGroup;
