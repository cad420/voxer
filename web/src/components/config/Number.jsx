import React, { Component } from 'react'

export default class extends Component {
  handleChange = (e) => {
    const { onChange, label, validator } = this.props
    const value = e.target.value
    if (validator && !validator(value)) {
      return
    }
    onChange && onChange(label, parseFloat(value))
  }

  render() {
    const { label, value, max = 500, min = -500 } = this.props
    return (
      <div>
        <label>
          {label}
          &nbsp;&nbsp;
          <input
            name={label}
            type="range"
            value={value || 0}
            max={max}
            min={min}
            onChange={this.handleChange}
          />
          &nbsp;&nbsp;
          {value}
        </label>
      </div>
    )
  }
}