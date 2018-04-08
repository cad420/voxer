import React, { Component } from 'react'
import { DragSource } from 'react-dnd'

const boxSource = {
  beginDrag(props, monitor, component) {
    const { id, left, top } = props
    props.setCurrent(id)
    return { id, left, top }
  }
}

class Box extends Component {
  render() {
    const {
      left,
      top,
      id,
      current,
      setCurrent,
      connectDragSource,
      isDragging,
      children
    } = this.props
    if (isDragging) return null
    return connectDragSource(
      <div
        className={'box' + (current === id ? ' current' : '')}
        style={{ left, top }}
        onClick={e => {
          e.stopPropagation()
          setCurrent(current === id ? '' : id)
        }}>
        {children}
      </div>
    )
  }
}

export default DragSource('box', boxSource, (connect, monitor) => ({
  connectDragSource: connect.dragSource(),
  isDragging: monitor.isDragging()
}))(Box)