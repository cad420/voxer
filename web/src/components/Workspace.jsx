import React, { Component } from 'react'
import { DiagramWidget, DefaultNodeModel } from 'storm-react-diagrams'
import { AdvancedPortModel } from '../manager'

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
    const data = JSON.parse(e.dataTransfer.getData('module-info'));
    const nodesCount = Object.keys(
      app.getDiagramEngine()
        .getDiagramModel()
        .getNodes()
    ).length;

    const node = new DefaultNodeModel(data.name + ' ' + (nodesCount + 1), '#333');
    node.extras.params = data.params
    node.extras.values = {}
    data.params.forEach(param => {
      if (param.default) {
        node.extras.values[param.label] = param.default
      }
    })
    data.ports.inputs.forEach(port => {
      node.addPort(new AdvancedPortModel(true, port.name, port.label));
    })
    data.ports.outputs.forEach(port => {
      node.addPort(new AdvancedPortModel(false, port.name, port.label));
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
        <DiagramWidget
          ref={diagram => this.diagram = diagram}
          maxNumberPointsPerLink={0}
          diagramEngine={app.getDiagramEngine()}
        />
      </section>
    )
  }
}
