import React from 'react'
import Module from './Module'
import modules from '../modules'

export default (props) => {
  const moduleCategories = Object.keys(modules)
  return (
    <section className="module-list">
      <h3 className="panel-title">Module List</h3>
      {moduleCategories.map(cate => {
        const common = modules[cate].common
        return (
          <div key={cate}>
            <p className="module-type">{cate}</p>
            {
              modules[cate].type.map(({ name, type, ports = {}, params = [], node = '' }) => {
                const _ports = Object.create(null)
                _ports.inputs = ((common.ports || {} ).inputs || []).concat(ports.inputs || [])
                _ports.outputs = ((common.ports || {} ).outputs || []).concat(ports.outputs || [])
                return (
                  <Module
                    key={name}
                    name={name}
                    type={type}
                    ports={_ports}
                    node={node}
                    params={(common.params || []).concat(params)}
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