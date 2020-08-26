import { DataTypes }from 'sequelize';
import sequelize from './index'

const Annotation = sequelize.define('Annotation', {
  dataset: {
    type: DataTypes.INTEGER,
    autoIncrement: true,
    primaryKey: true
  },
  tag: {
    type: DataTypes.INTEGER,
    
  },

})

export default Annotation;