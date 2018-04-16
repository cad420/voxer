import { DiagramEngine, DiagramModel } from 'storm-react-diagrams';
import VovisNodeModel from './components/diagram/node/Model'
import VovisNodeFactory from './components/diagram/node/Factory'
import VovisPortFactory from './components/diagram/port/Factory'
import VovisLinkFactory from './components/diagram/link/Factory'

export default class Manager {
	constructor(onSelectionChanged) {
		this.diagramEngine = new DiagramEngine();
		this.diagramEngine.installDefaultFactories();
		this.onSelectionChanged = onSelectionChanged;
		this.newModel();
	}

	newModel() {
		this.activeModel = new DiagramModel();
		this.diagramEngine.setDiagramModel(this.activeModel);
		this.diagramEngine.registerNodeFactory(new VovisNodeFactory());
		this.diagramEngine.registerLinkFactory(new VovisLinkFactory());
		this.diagramEngine.registerPortFactory(new VovisPortFactory());

		let node1 = new VovisNodeModel('Dataset 1', '#333');
		let port = node1.addInPort('Out1', 'Out');
		node1.setPosition(300, 100);

		let node2 = new VovisNodeModel('Transfer Function 2', '#333');
		node2.addOutPort('Out1', 'Out');
		node2.setPosition(200, 150);

		let node3 = new VovisNodeModel('Volume 3', '#333');
		let port3 = node3.addInPort('In', 'In');
		node3.addInPort('In2', 'In');
		node3.addOutPort('Out', 'Out');
		node3.setPosition(400, 300);

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
