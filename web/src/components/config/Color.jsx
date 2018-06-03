import React, { Component } from 'react'

export default class extends Component {
  handleChange = (e) => {
    const { onChange, label } = this.props
    let color = e.target.value
    console.log(color)
    if (color.length === 4) {
      color = '#' + color[1] + color[1] + color[2] + color[2] + color[3] + color[3];
    }
    onChange && onChange(label, color)
  }

  render() {
    const { label, value } = this.props
    return (
      <div>
        <label>
          {label}
          &nbsp;&nbsp;
          <input
            type="color"
            value={value}
            onChange={this.handleChange}
          />
          &nbsp;&nbsp;
          {value}
        </label>
      </div>
    )
  }
}