import React, { Component } from 'react'

export default class extends Component {
  handleChange = (e) => {
    const { onChange, label } = this.props
    const value = e.target.value
    onChange && onChange(label, value === 'true')
  }

  render() {
    const { label, value } = this.props
    return (
      <div>
        {label}
        &nbsp;&nbsp;
        <label>
          <input
            name={label}
            type="radio"
            value={true}
            checked={value === true}
            onChange={this.handleChange}
          />
          ON
        </label>
        &nbsp;&nbsp;
        <label>
          <input
            name={label}
            type="radio"
            value={false}
            checked={value === false}
            onChange={this.handleChange}
          />
          OFF
        </label>
      </div>
    )
  }
}