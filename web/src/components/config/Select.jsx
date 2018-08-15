import React, { Component } from 'react'
import { Select } from 'antd';

const { Option } = Select;

export default class extends Component {
  handleChange = value => {
    const { onChange, label } = this.props;
    onChange && onChange(label, value);
  }

  render() {
    const { label, options = [], value = '' } = this.props;
    return (
      <div>
        <div><b>{label}</b></div>
        <Select value={value} onChange={this.handleChange} style={{ width: '60%' }}>
          {
            options.map(option => (
              typeof option === 'object' ?
              <Option key={option.value}>{option.label}</Option> :
              <Option key={option}>{option}</Option> 
            ))
          }
        </Select>
      </div>
    )
  }
}