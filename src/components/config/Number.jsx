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
    const { label, value } = this.props
    return (
      <div>
        <label>
          {label}
          &nbsp;&nbsp;
          <input
            name={label}
            type="number"
            value={value || 0}
            onChange={this.handleChange}
          />
        </label>
      </div>
    )
  }
}