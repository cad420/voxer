import React, { Component } from 'react'
import Float from './Float'

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
        <div>{label}</div>
        <div>{
          ['x', 'y', 'z'].map((item, i) => (
            <Float
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