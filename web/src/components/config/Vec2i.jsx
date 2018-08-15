import React, { Component } from 'react'
import Int from './Int'

const map = {
  x: 0, y: 1, z: 2
}

export default class extends Component {
  handleChange = (label, value) => {
    const { onChange } = this.props
    const res = (this.props.value || []).slice()
    res[map[label]] = value
    onChange && onChange(this.props.label, res)
  }

  render() {
    const { label, value = [], max, min  } = this.props
    return (
      <div>
        <div><b>{label}</b></div>
        <div>{
          ['x', 'y'].map((item, i) => (
            <Int
              key={item}
              label={item}
              max={max} min={min}              
              value={value[i] || 0 }
              onChange={this.handleChange}
            />
          ))
        }</div>
      </div>
    )
  }
}