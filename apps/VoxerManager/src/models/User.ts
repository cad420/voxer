import { ObjectID } from "mongodb";

interface Permission {
  users: {
    create?: boolean;
    delete?: boolean;
    read?: boolean;
    update?: boolean;
  };
  groups: {
    create?: boolean;
    delete?: boolean;
    read?: boolean;
    update?: boolean;
  };
  group: Record<
    string,
    {
      delete?: boolean;
      read?: boolean;
      update?: boolean;
      addUsers?: boolean;
      deleteUsers?: boolean;
      createDatasets?: boolean;
      deleteDatasets?: boolean;
      updateDatasets?: boolean;
    }
  >;
}

export interface IUser {
  _id: ObjectID;
  name: string;
  password: string;
  permission: Permission;
}

export type IUserWithoutPwd = Omit<IUser, "password">;

export type IUserCreateInfo = Omit<IUser, "_id">;

export interface IUserFrontEnd {
  id?: string;
  name: string;
  permission: Permission;
}

export interface IUserBackend extends IUserFrontEnd {}
