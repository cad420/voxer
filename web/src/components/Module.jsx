import React, { Component } from 'react'

export default class Module extends Component {
  render() {
    return (
      <div
        draggable={true}
        onDragStart={e => {
          console.log(this.props)
          e.dataTransfer.setData('module-info', JSON.stringify({...this.props}))
        }}
        className="module-option">
        {this.props.name}
      </div>
    )
  }
}
