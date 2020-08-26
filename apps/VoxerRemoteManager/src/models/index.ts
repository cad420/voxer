import { Sequelize }from 'sequelize';

const sequelize = new Sequelize({
  dialect: 'sqlite',
  storage: 'path/to/database.sqlite'
});

export default sequelize;
