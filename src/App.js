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
      current: '',
      config: {}
    }
    this.app = new Manager(this.handleSelectionChange);
  }

  handleSelectionChange = (data) => {
    console.log(data)
    this.setState({
      current: data.isSelected ? data.entity.id : '',
      config: data.extra.panel,
    })
  }

  render() {
    const { current, config } = this.state
    return (
      <div className="App">
        <header className="App-header">
          <h1>Volume Visualization</h1>
        </header>
        <div className="App-container">
          <ModuleList />
          <Workspace app={this.app} />
          <Config current={current} config={config} />
        </div>
      </div>
    );
  }
}
