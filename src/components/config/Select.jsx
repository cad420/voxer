import React, { Component } from 'react'

export default class extends Component {
  render() {
    const { select } = this.props
    return (
      <div>
        <label htmlFor={label}>Source</label>
        <select id={label} name={label} defaultValue="">
          <option value="">Please choose</option>
          <option value="tooth">tooth</option>
          <option value="bucky">bucky</option>
          <option value="Other">Other</option>
        </select>
      </div>
    )
  }
}