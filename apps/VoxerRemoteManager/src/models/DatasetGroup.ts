import { ObjectID } from "mongodb";
import { AnnotationType, DatasetAnnotations } from "./Annotation";

export type LabelId = number;

type DatasetGroup = {
  _id: ObjectID;
  name: string;
  labels: Array<{
    id: LabelId;
    name: string;
    color: string;
    type: AnnotationType;
  }>;
  datasets: Record<
    string,
    {
      name: string;
      labels: Record<LabelId, DatasetAnnotations>;
    }
  >;
};

export default DatasetGroup;
