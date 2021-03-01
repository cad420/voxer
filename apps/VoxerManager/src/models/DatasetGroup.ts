import { ObjectID } from "mongodb";
import { AnnotationType, DatasetAnnotations } from "./Annotation";

export type LabelId = ObjectID;

enum Application {
  Annotation = "annotation",
  Pipeline = "pipeline",
}

export type Label = {
  id: LabelId;
  name: string;
  color: string;
  type: AnnotationType;
};

type IGroupShared = {
  name: string;
  creator: string;
  createTime: number;
  datasets: Record<
    string,
    {
      name: string;
      labels: Record<string, DatasetAnnotations>;
    }
  >;
  users: string[];
  applications: Application[];
  labels?: Array<Label>;
};

type IGroupBackend = IGroupShared;

interface IGroupFrontEnd extends IGroupShared {
  id: string;
}

type DatasetGroup = {
  id: string;
  name: string;
  creator: string;
  createTime: number;
  datasets: Record<
    string,
    {
      name: string;
      labels: Record<string, DatasetAnnotations>;
    }
  >;
  users: string[];
  applications: Application[];
  labels?: Array<Label>;
};

export default DatasetGroup;

export { IGroupShared, IGroupFrontEnd, IGroupBackend };
