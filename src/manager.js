import * as SRD from "storm-react-diagrams";
import React from 'react';
import {
	DefaultLinkModel,
	DefaultPortModel,
	DefaultLinkFactory
} from 'storm-react-diagrams';

class AdvancedLinkModel extends DefaultLinkModel {
	constructor() {
		super("advanced");
		this.setColor('#ddd');
	}
}

export class AdvancedPortModel extends DefaultPortModel {
	createLinkModel() {
		return new AdvancedLinkModel();
	}
}

class AdvancedLinkFactory extends DefaultLinkFactory {
	constructor() {
		super();
		this.type = "advanced";
	}

	getNewInstance(initialConfig) {
		return new AdvancedLinkModel();
	}

	generateLinkSegment(model, widget, selected, path) {
		return (
			<g>
				<path
					className={selected ? widget.bem("--path-selected") : ""}
					strokeWidth={model.width}
					stroke={model.color}
					d={path} />
			</g>
		);
	}
}

export default class Manager {
	constructor(onSelectionChanged) {
		this.diagramEngine = new SRD.DiagramEngine();
		this.diagramEngine.installDefaultFactories();
		this.onSelectionChanged = onSelectionChanged;
		this.newModel();
	}

	newModel() {
		this.activeModel = new SRD.DiagramModel();
		this.diagramEngine.setDiagramModel(this.activeModel);
		this.diagramEngine.registerLinkFactory(new AdvancedLinkFactory());

		//3-A) create a default node
		let node1 = new SRD.DefaultNodeModel("Dataset 1", "#333");
		let port = node1.addPort(new AdvancedPortModel(false, 'Out1', "Out"));
		node1.setPosition(300, 100);

		//3-B) create another default node
		let node2 = new SRD.DefaultNodeModel("Transfer Function 2", "#333");
		node2.addPort(new AdvancedPortModel(false, "Out1", "Out"));
		node2.setPosition(200, 150);

		let node3 = new SRD.DefaultNodeModel("Volume 3", "#333");
		let port3 = node3.addPort(new AdvancedPortModel(true, "In", "Int"));
		node3.addPort(new AdvancedPortModel(true, "In2", "In"));
		node3.addPort(new AdvancedPortModel(false, "Out", "Out"));
		node3.setPosition(400, 300);

		// link the ports
		let link1 = port.link(port3);

		this.activeModel.addAll(node1, node2, node3, link1);
		[node1, node2, node3].forEach(item => {
			item.addListener({
				selectionChanged: this.onSelectionChanged
			});
		});
	}

	getActiveDiagram() {
		return this.activeModel;
	}

	getDiagramEngine() {
		return this.diagramEngine;
	}

	addModule(node) {
		node.addListener({
			selectionChanged: this.onSelectionChanged
		})
		this.diagramEngine
      .getDiagramModel()
      .addNode(node);
	}
}
