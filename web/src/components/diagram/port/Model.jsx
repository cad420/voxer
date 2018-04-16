import { DefaultPortModel } from 'storm-react-diagrams'
import LinkModel from '../link/Model'

export default class Model extends DefaultPortModel {
	constructor(isInput, name, label, id) {
		super(isInput, name, label, id)
		this.type = 'vovis'
	}

	createLinkModel() {
		return new LinkModel();
	}

	canLinkToPort(port) {
		return this.name === port.name
	}
}