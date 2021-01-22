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

interface User {
  id?: string;
  name: string;
  password?: string;
  permission: Permission;
}

export default User;
