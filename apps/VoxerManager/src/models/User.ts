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

interface IUserFrontEnd {
  id?: string;
  name: string;
  permission: Permission;
}

interface IUserBackend extends IUserFrontEnd {
  password: string;
}

export { IUserFrontEnd, IUserBackend };
