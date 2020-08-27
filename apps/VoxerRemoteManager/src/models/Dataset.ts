import { DataTypes, Model, Optional } from "sequelize";
import sequelize from "./index";

interface DatasetAttributes {
  id: number;
  name: string;
  variable: string;
  timestep: number;
  path: string;
}

type DatasetCreationAttributes = Optional<DatasetAttributes, "id">;

class Dataset
  extends Model<DatasetAttributes, DatasetCreationAttributes>
  implements DatasetAttributes {
  id: number;
  name: string;
  variable: string;
  timestep: number;
  path: string;

  public readonly createdAt: Date;
  public readonly updatedAt: Date;
}

Dataset.init(
  {
    id: {
      type: DataTypes.INTEGER,
      autoIncrement: true,
      primaryKey: true,
    },
    name: {
      type: DataTypes.STRING,
      allowNull: false,
    },
    variable: {
      type: DataTypes.STRING,
      defaultValue: "default",
    },
    timestep: {
      type: DataTypes.INTEGER,
      defaultValue: 0,
    },
    path: {
      type: DataTypes.STRING,
      allowNull: false,
    },
  },
  {
    sequelize,
    tableName: "datasets",
  }
);

export default Dataset;
