import React, { Component } from 'react';
import { Switch } from 'antd';

export default class extends Component {
  handleChange = (checked) => {
    const { onChange, label } = this.props
    onChange && onChange(label, checked)
  }

  render() {
    const { label, value } = this.props
    return (
      <div>
        <div><b>{label}</b></div>
          <Switch
            value={value}
            onChange={this.handleChange}
          />
      </div>
    )
  }
}