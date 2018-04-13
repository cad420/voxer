import React, { Component } from 'react'
import Int from './Int'

const map = {
  r: 0, g: 1, b: 2, a: 3
}

export default class extends Component {
  handleChange = (label, value) => {
    const { onChange } = this.props
    const res = (this.props.value || []).slice()
    res[map[label]] = value
    onChange && onChange(this.props.label, res)
  }

  render() {
    const { label, value = [] } = this.props
    return (
      <div>
        {label}
        &nbsp;&nbsp;
        {
          ['x', 'y', 'z'].map((item, i) => (
            <Int
              key={item}
              label={item}
              value={value[i] || 0 }
              onChange={this.handleChange}
            />
          ))
        }
      </div>
    )
  }
}