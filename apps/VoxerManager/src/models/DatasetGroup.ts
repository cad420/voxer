import { ObjectID } from "mongodb";
import { AnnotationType, DatasetAnnotations } from "./Annotation";

export type LabelId = ObjectID;

export type Label = {
  id: LabelId;
  name: string;
  color: string;
  type: AnnotationType;
};

type DatasetGroup = {
  _id: ObjectID;
  name: string;
  labels: Array<Label>;
  datasets: Record<
    string,
    {
      name: string;
      labels: Record<string, DatasetAnnotations>;
    }
  >;
};

export default DatasetGroup;
