import React, { Component } from 'react';
import Manager from './manager';
import ModuleList from './components/ModuleList';
import Workspace from './components/Workspace';
import Config from './components/Config';
import './App.css';
import '../node_modules/storm-react-diagrams/dist/style.min.css';

export default class App extends Component {
  constructor(props) {
    super(props)
    this.state = {
      current: null,
      params: {},
      values: {}
    }
    window.app = this.app = new Manager();
    this.app.on('selectionChanged', this.handleSelectionChange)
  }

  handleSelectionChange = (data) => {
    const { entity } = data
    if (data.isSelected) {
      console.log(entity)
      this.setState({
        current: entity,
        values: entity.extras.values || {}
      })
    } else {
      this.setState({
        current: null,
        params: [],
        values: null
      })
    }
  }

  handleConfigChange = (label, value) => {
    const { current, values } = this.state
    this.setState({
      values: {
        ...values,
        [label]: value
      }
    })
    current.extras.values[label] = value
    this.update()
  }

  update = () => {
    this.app.displays.forEach((display, i) => {
      if (!display.el) {
        this.app.displays.splice(i, 1)
      } else {
        display.el.renderImage()
      }
    })
  }

  render() {
    const { current, values } = this.state
    return (
      <div className="App">
        <header className="App-header">
          <h1>Volume Visualization</h1>
        </header>
        <div className="App-container">
          <ModuleList />
          <Workspace app={this.app} />
          <Config
            current={current}
            values={values}
            onChange={this.handleConfigChange}
          />
        </div>
      </div>
    );
  }
}
