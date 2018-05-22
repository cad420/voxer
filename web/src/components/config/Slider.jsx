import React, { Component } from 'react';
import { Slider, InputNumber, Row, Col } from 'antd';

export default class extends Component {
  onChange = value => {
    const { label, onChange } = this.props;
    onChange && onChange(label, value);
  }

  render() {
    const { label, value, min = 0, max = 100, step = 1 } = this.props;
    return (
      <Row gutter={16}>
        <Col span={6}>
          <b>{label}</b>
        </Col>
        <Col span={12}>
          <Slider
            min={min}
            max={max}
            step={step}
            onChange={this.onChange}
            value={value}
          />
        </Col>
        <Col span={6}>
          <InputNumber
            style={{ width: '100%' }}
            min={min}
            max={max}
            onChange={this.onChange}
            value={value}
          />
        </Col>
      </Row>
    );
  }
}