import { DiagramEngine } from 'storm-react-diagrams';
import VovisDiagramModel from './components/diagram/Model'
import VovisNodeFactory from './components/diagram/node/Factory'
import VovisPortFactory from './components/diagram/port/Factory'
import VovisLinkFactory from './components/diagram/link/Factory'
import DisplayNodeFactory from './components/diagram/displayNode/Factory'
import save from './save'

export default class Manager {
	constructor(onSelectionChanged) {
		this.diagramEngine = new DiagramEngine();
		this.displays = [];
		this.diagramEngine.installDefaultFactories();
		this.onSelectionChanged = onSelectionChanged;
		this.newModel();
	}

	newModel() {
		this.activeModel = new VovisDiagramModel();
		this.diagramEngine.setDiagramModel(this.activeModel);
		this.diagramEngine.registerNodeFactory(new DisplayNodeFactory());
		this.diagramEngine.registerNodeFactory(new VovisNodeFactory());
		this.diagramEngine.registerLinkFactory(new VovisLinkFactory());
		this.diagramEngine.registerPortFactory(new VovisPortFactory());

		this.activeModel.deSerializeDiagram(JSON.parse(save), this.diagramEngine)

		const nodes = this.activeModel.getNodes()
		Object.keys(nodes).forEach(id => {
			if (nodes[id].type === 'display') {
				this.displays.push(nodes[id])
			}
			nodes[id].addListener({
				selectionChanged: this.onSelectionChanged
			})
		})
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
