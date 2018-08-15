import React from 'react';

const Config = React.createContext({
  server: 'localhost:3000/',
  update: () => {},
});

export default Config;
