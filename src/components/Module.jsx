import React, { Component } from 'react'

export default class Module extends Component {
  render() {
    const { name, ports, panel } = this.props
    return (
      <div
        draggable={true}
        onDragStart={e => {
          e.dataTransfer.setData('module-info', JSON.stringify({
            ports, name, panel
          }))
        }}
        className="module-option">
        {name}
      </div>
    )
  }
}
