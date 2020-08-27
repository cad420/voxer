import { Sequelize }from 'sequelize';
import { DATABASE_PATH } from '../config'

const sequelize = new Sequelize({
  dialect: 'sqlite',
  storage: DATABASE_PATH
});

export default sequelize;
