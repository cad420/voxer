import React, { Component } from 'react';
import { DragDropContext } from 'react-dnd';
import HTML5Backend from 'react-dnd-html5-backend';
import Workspace from './components/Workspace'
import Module from './components/Module'
import './App.css';

export default class App extends Component {
  render() {
    return (
      <div className="App">
        <header className="App-header">
          <h1 className="App-title">Volume Visualization</h1>
        </header>
        <div className="container">
          {
            DragDropContext(HTML5Backend)(() => (
              <div>
                <section className="module-list">
                  <Module>Dataset</Module>
                  <Module>Transfer Function</Module>
                  <Module>Volume</Module>
                  <Module>Geometry</Module>
                  <Module>Light</Module>
                  <Module>Camera</Module>
                  <Module>Model</Module>
                  <Module>Transition</Module>
                  <Module>Renderer</Module>
                </section>
                <Workspace />
              </div>
            ))
          }
          <section className="config-panel">
            <h3>Config</h3>
            <label for="title">Dataset</label>
            &nbsp;&nbsp;&nbsp;
            <select id="title" name="title">
              <option value="" selected>Please choose</option>
              <option value="tooth">tooth</option>
              <option value="bucky">bucky</option>
              <option value="Other">Other</option>
            </select>
            <br/>
            <br/>
            <div>
              <input type="radio" name="a" value="1" /><label for="1">1</label>
              <input type="radio" name="a" value="2" /><label for="2">2</label>
            </div>
            <br/>
          </section>
        </div>
      </div>
    );
  }
}
