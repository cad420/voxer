import React from 'react'
import Module from './Module'
import modules from '../modules'

export default (props) => {
  const moduleTypes = Object.keys(modules)
  return (
    <section className="module-list">
      {moduleTypes.map(type => {
        const common = modules[type].common
        return (
          <div key={type}>
            <p className="module-type">{type}</p>
            {
              modules[type].type.map(({ name, ports = [], panel = [] }) => {
                return (
                  <Module
                    key={name}
                    name={name}
                    ports={(common.ports || []).concat(ports)}
                    panel={(common.panel || []).concat(panel)}
                  />
                )
              })
            }
          </div>
        )
      })}
    </section>
  )
}