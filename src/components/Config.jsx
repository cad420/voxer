import React, { Component } from 'react'

export default class Config extends Component {
  render() {
    const { current, config } = this.props
    return (
      <section className="config">
        <h3 className="panel-title">Config</h3>
        {
          current &&
          JSON.stringify(config)
        }
      </section>
    )
  }
}
