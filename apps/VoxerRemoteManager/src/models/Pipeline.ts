import { DataTypes, Model, Optional } from "sequelize";
import sequelize from "./index";

interface PipelineAttributes {
  id: number;
  comment: string;
  params: string;
}

type PipelineCreationAttributes = Optional<PipelineAttributes, "id" | "comment">;

class Pipeline
  extends Model<PipelineAttributes, PipelineCreationAttributes>
  implements PipelineAttributes {
  id: number;
  comment: string;
  params: string;

  public readonly createdAt: Date;
  public readonly updatedAt: Date;
}

Pipeline.init(
  {
    id: {
      type: DataTypes.INTEGER,
      autoIncrement: true,
      primaryKey: true,
    },
    comment: {
      type: DataTypes.STRING,
      defaultValue: ''
    },
    params: {
      type: DataTypes.TEXT,
      allowNull: false,
    },
  },
  {
    sequelize,
    tableName: "pipelines",
  }
);

export default Pipeline;
