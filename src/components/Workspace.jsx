import React, { Component } from 'react'
import { DropTarget } from 'react-dnd'
import Box from './Box'

const boxTarget = {
  drop(props, monitor, component) {
    const item = monitor.getItem()
    const type = monitor.getItemType()
    if (type === 'module') {
      console.log(item)
      const pos = monitor.getClientOffset();
      component.createBox(item.children, pos.x - 250, pos.y - 60)
      return
    }
    const delta = monitor.getDifferenceFromInitialOffset()
    const left = Math.round(item.left + delta.x)
    const top = Math.round(item.top + delta.y)

    component.moveBox(item.id, left, top)
  }
}

class Workspace extends Component {
  constructor(props) {
    super(props)
    this.state = {
      boxes: {
        a: { top: 20, left: 280, title: 'Dataset' },
        b: { top: 180, left: 220, title: 'Volume' }
      },
      current: ''
    }
  }

  setCurrent = (id) => {
    const { current } = this.state
    this.setState({ current: id })
  }

  moveBox(id, left, top) {
    const { boxes } = this.state
    this.setState({
      boxes: {
        ...boxes,
        [id]: { ...boxes[id], left, top }
      }
    })
  }

  createBox(title, left, top) {
    const { boxes } = this.state
    const id = Math.random().toString()
    this.setState({
      current: id,
      boxes: {
        ...boxes,
        [id]: { left, top, title }
      }
    })
  }

  render() {
    const { connectDropTarget } = this.props
    const { boxes, current } = this.state

    return connectDropTarget(
      <div className="workspace" onClick={() => this.setCurrent('')}>
        {Object.keys(boxes).map(key => {
          const { left, top, title } = boxes[key]
          return (
            <Box
              current={current}
              setCurrent={this.setCurrent}
              key={key}
              id={key}
              left={left}
              top={top}
            >
              {title}
            </Box>
          )
        })}
      </div>,
    )
  }
}

export default DropTarget(['box', 'module'], boxTarget, connect => ({
  connectDropTarget: connect.dropTarget()
}))(Workspace)