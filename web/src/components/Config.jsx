import React, { Component } from 'react'
import DataEntries from './config/index'
import modules from '../modules'

export default class Config extends Component {
  render() {
    const { current, values = {}, onChange } = this.props
    if (!current) {
      return <section className="config" />
    }
    const category = modules[current.extras.category]
    let typeParams = []
    for (let i = 0; i < category.type.length; i++) {
      if (category.type[i].name === current.extras.name) {
        typeParams = category.type[i].params || []
        break;
      }
    }
    const params = (category.common.params || []).concat(typeParams)
    return (
      <section className="config">
        <h3 className="panel-title">Config</h3>
        {
          params.map(param => {
            const Entry = DataEntries[param.type]
            if (!Entry) return JSON.stringify(param)
            return (
              <div className="config-item" key={param.label}>
              <Entry
                {...param}
                value={values[param.label]}
                onChange={onChange}
              />
              </div>
            )
          })
        }
      </section>
    )
  }
}
