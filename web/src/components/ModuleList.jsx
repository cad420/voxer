import React, { Component } from 'react'
import modules from '../modules'

class Module extends Component {
  constructor(props) {
    super(props)
    this.state = { onDragging: false }
  }

  render() {
    return (
      <div
        draggable={true}
        onDragStart={e => {
          e.dataTransfer.setData('module-info', JSON.stringify({...this.props}))
        }}
        className="module-option">
        {this.props.name}
      </div>
    )
  }
}


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
                    category={cate}
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