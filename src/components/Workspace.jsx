import React, { Component } from 'react'
import { DiagramWidget, DefaultNodeModel } from 'storm-react-diagrams'
import { AdvancedPortModel } from '../manager'

export default class Workspace extends Component {
  render() {
    const { app } = this.props
    return (
      <section
        className="workspace"
        onDrop={e => {
          const data = JSON.parse(e.dataTransfer.getData('module-info'));
          const nodesCount = Object.keys(
            app.getDiagramEngine()
              .getDiagramModel()
              .getNodes()
          ).length;

          const node = new DefaultNodeModel(data.name + ' ' + (nodesCount + 1), '#333');
          node.extras.panel = data.panel
          node.extras.values = {}
          data.ports.forEach(port => {
            node.addPort(new AdvancedPortModel(port.isInput, port.name, port.label));
          })
          const points = app.getDiagramEngine().getRelativeMousePoint(e);
          node.x = points.x;
          node.y = points.y;
          app.addModule(node)
          this.forceUpdate();
        }}
        onDragOver={e => e.preventDefault()}
      >
        <DiagramWidget maxNumberPointsPerLink={0} diagramEngine={app.getDiagramEngine()} />
      </section>
    )
  }
}
