import React, { Component } from 'react';
import { hot } from 'react-hot-loader';
import Manager from './manager';
import Header from './components/Header';
import ModuleList from './components/ModuleList';
import Workspace from './components/Workspace';
import Config from './components/Config';
import ConfigContext from './store/config';
import './styles/App.css';
import '../node_modules/storm-react-diagrams/dist/style.min.css';

class App extends Component {
  constructor(props) {
    super(props)
    this.state = {
      current: null,
      params: {},
      values: {},
      config: {
        server: 'localhost:3000',
        update: this.updateConfig
      }
    }
    window.app = this.app = new Manager();
    this.app.on('selectionChanged', this.handleSelectionChange)
  }

  updateConfig = (key, value) => {
    this.setState({
      config: {
        ...this.state.config,
        [key]: value
      }
    })
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
    const { current, values, config } = this.state
    return (
      <ConfigContext.Provider value={config}>
        <div className="App">
          <Header />
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
      </ConfigContext.Provider>
    );
  }
}

export default hot(module)(App);
