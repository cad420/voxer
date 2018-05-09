import React, { Component } from 'react'
import DataEntries from './config/index'

export default class Config extends Component {
  render() {
    const { current, params = [], values = {}, onChange } = this.props
    return (
      <section className="config">
        <h3 className="panel-title">Config</h3>
        {
          current &&
          params.map(param => {
            const Entry = DataEntries[param.type]
            if (!Entry) return JSON.stringify(param)
            return (
              <div className="config-item" key={param.label}>
              <Entry
                {...param}
                value={values[param.label] || param.default}
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
