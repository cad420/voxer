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
    this.app = new Manager(this.handleSelectionChange);
  }

  handleSelectionChange = (data) => {
    const { entity } = data
    if (data.isSelected) {
      console.log(entity)
      this.setState({
        current: entity,
        params: entity.extras.params,
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
    this.app.displays.forEach(display => {
      const data = display.extras.values
      if (!data.image) return
      const dataset = data.image.model ? (
        data.image.model.volume ?
        (
          data.image.model.volume.dataset ?
          data.image.model.volume.dataset.source :
          undefined
        ) : undefined
      ) : undefined;
      display.el.renderImage(dataset, null, null, null, null, data.camera, null, data.size)
    })
  }

  render() {
    const { current, params, values } = this.state
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
            params={params}
            values={values}
            onChange={this.handleConfigChange}
          />
        </div>
      </div>
    );
  }
}
