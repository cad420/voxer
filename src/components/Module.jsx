import React, { Component } from 'react'
import { DragSource } from 'react-dnd'

const boxSource = {
  beginDrag(props) {
    const { children } = props
    return { children }
  }
}

class Module extends Component {
  render() {
    const {
      connectDragSource,
      children
    } = this.props
    return connectDragSource(
      <div className="module">{children}</div>,
    )
  }
}

export default DragSource('module', boxSource, (connect, monitor) => ({
  connectDragSource: connect.dragSource()
}))(Module)