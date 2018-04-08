import React, { Component } from 'react'
import { DragSource } from 'react-dnd'

const boxSource = {
  beginDrag(props) {
    const { id, left, top } = props
    console.log(id)
    return { id, left, top }
  }
}

class Box extends Component {
  render() {
    const {
      left,
      top,
      connectDragSource,
      isDragging,
      children
    } = this.props
    if (isDragging) {
      return null
    }
    return connectDragSource(
      <div className="box" style={{ left, top }}>{children}</div>,
    )
  }
}

export default DragSource('box', boxSource, (connect, monitor) => ({
  connectDragSource: connect.dragSource(),
  isDragging: monitor.isDragging()
}))(Box)