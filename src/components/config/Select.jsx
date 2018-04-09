import React, { Component } from 'react'

export default class extends Component {
  render() {
    return (
      <div>
        <label htmlFor="title">Source</label>
        <select id="title" name="title" defaultValue="">
          <option value="">Please choose</option>
          <option value="tooth">tooth</option>
          <option value="bucky">bucky</option>
          <option value="Other">Other</option>
        </select>
      </div>
    )
  }
}