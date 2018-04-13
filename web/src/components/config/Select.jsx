import React, { Component } from 'react'

export default class extends Component {
  handleChange = e => {
    const { onChange, label } = this.props
    onChange && onChange(label, e.target.value)
  }

  render() {
    const { label, options = [], value = '' } = this.props
    return (
      <div>
        <label htmlFor={label}>Source</label>
        <select id={label} name={label} value={value} onChange={this.handleChange}>
          <option value="">Please choose</option>
          {
            options.map(option => (
              <option key={option} value={option}>{option}</option>
            ))
          }
        </select>
      </div>
    )
  }
}