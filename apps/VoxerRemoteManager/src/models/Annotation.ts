import { DataTypes, Model, Optional } from "sequelize";
import sequelize from "./index";

interface AnnotationAttributes {
  id: number;
  dataset: number;
  axis: string;
  index: number;
  tag: number;
  comment: string;
  type: string;
  coordinates?: string;
}

type AnnotationCreationAttributes = Optional<
  AnnotationAttributes,
  "id" | "comment"
>;

class Annotation
  extends Model<AnnotationAttributes, AnnotationCreationAttributes>
  implements AnnotationAttributes {
  id: number;
  dataset: number;
  axis: string;
  index: number;
  tag: number;
  comment: string;
  type: string;
  coordinates?: string;

  public readonly createdAt: Date;
  public readonly updatedAt: Date;
}

Annotation.init(
  {
    id: {
      type: DataTypes.INTEGER,
      autoIncrement: true,
      primaryKey: true,
    },
    dataset: {
      type: DataTypes.INTEGER,
      allowNull: false,
    },
    axis: {
      type: DataTypes.STRING,
      allowNull: false
    },
    index: {
      type: DataTypes.INTEGER,
      allowNull: false
    },
    tag: {
      type: DataTypes.INTEGER,
      allowNull: false,
    },
    comment: {
      type: DataTypes.STRING,
      defaultValue: "",
    },
    type: {
      type: DataTypes.STRING,
      allowNull: false,
    },
    coordinates: {
      type: DataTypes.TEXT,
      allowNull: false,
    },
  },
  {
    sequelize,
    tableName: "annotations",
    indexes: [
      {
        unique: true,
        fields: ["dataset"],
      },
    ],
  }
);

export default Annotation;
