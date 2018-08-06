import React, { Component } from 'react'
import VovisDiagramWidget from '../components/diagram/Widget'
import DisplayNodeModel from '../components/diagram/displayNode/Model'
import VovisNodeModel from '../components/diagram/node/Model'
import VovisPortModel from '../components/diagram/port/Model'

export default class Workspace extends Component {
  componentDidMount() {
    window.removeEventListener('keyup', this.diagram.onKeyUpPointer)
    const workspace = document.getElementById('workspace')
    workspace.onmouseover = workspace.focus
    workspace.onmouseout = workspace.blur
    workspace.onkeydown = this.diagram.onKeyUpPointer
  }

  handleDrop = e => {
    const { app } = this.props
    const dropped = e.dataTransfer.getData('module-info')
    if (!dropped) return
    const data = JSON.parse(dropped);
    let node = null
    if (data.node && data.node === 'image') {
      node = new DisplayNodeModel(data.name, '#333', 'image');
      app.displays.push(node)
    } else if (data.node && data.node === 'animation') {
      node = new DisplayNodeModel(data.name, '#333', 'animation');
      app.displays.push(node)
    } else {
      node = new VovisNodeModel(data.name, '#333');
    }
    node.extras.name = data.name
    node.extras.category = data.category
    node.extras.values = { type: data.type }
    node.extras.children = {}
    data.params.forEach(param => {
      if (param.default) {
        node.extras.values[param.label] = param.default
      }
    })
    data.ports.inputs.forEach(port => {
      node.addPort(new VovisPortModel(true, port.name, port.accepts, port.repeatable, port.required));
      node.extras.children[port.name] = false
    })
    data.ports.outputs.forEach(port => {
      node.addPort(new VovisPortModel(false, port.name, port.type, port.repeatable));
    })
    const points = app.getDiagramEngine().getRelativeMousePoint(e);
    node.x = points.x;
    node.y = points.y;
    app.addModule(node)
    this.forceUpdate();
  }

  handleDragOver = e => e.preventDefault()

  render() {
    const { app } = this.props
    return (
      <section
        id="workspace"
        tabIndex="0"
        className="workspace"
        onDrop={this.handleDrop}
        onDragOver={this.handleDragOver}
      >
        <VovisDiagramWidget
          ref={diagram => this.diagram = diagram}
          maxNumberPointsPerLink={0}
          diagramEngine={app.getDiagramEngine()}
        />
      </section>
    )
  }
}
