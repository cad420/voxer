interface User {
  id?: string;
  name: string;
  password?: string;
  permission: {
    platform: {
      createUsers?: boolean;
      deleteUsers?: boolean;
      readUsers?: boolean;
      updateUsers?: boolean;
      createGroups?: boolean;
      deleteGroups?: boolean;
      readGroups?: boolean;
      updateGroups?: boolean;
    };
    groups: Record<
      string,
      {
        delete?: boolean;
        read?: boolean;
        update?: boolean;
        createUsers?: boolean;
        deleteUsers?: boolean;
        readUsers?: boolean;
        updateUsers?: boolean;
        createDatasets?: boolean;
        deleteDatasets?: boolean;
        readDatasets?: boolean;
        updateDatasets?: boolean;
      }
    >;
  };
}

export default User;
