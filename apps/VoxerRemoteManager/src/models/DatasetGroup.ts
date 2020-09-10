import { ObjectID } from "mongodb";
import { AnnotationType, DatasetAnnotations } from "./Annotation";

type DatasetGroup = {
  _id: ObjectID;
  name: string;
  labels: Array<{
    name: string;
    color: string;
    type: AnnotationType;
    annotations: Record<string, DatasetAnnotations>;
  }>;
  datasets: Array<{
    id: ObjectID;
    name: string;
  }>;
};

export default DatasetGroup;
