import { ObjectID } from "mongodb";

interface Label {
  _id: ObjectID;
  group: ObjectID;
  dataset: ObjectID;
  name: string;
  color: string;
  type: string;
}

export default Label;
