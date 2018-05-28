import { DefaultPortModel } from 'storm-react-diagrams'
import LinkModel from '../link/Model'

export default class Model extends DefaultPortModel {
	constructor(isInput, name, label, repeatable, required = true) {
		super(isInput, name, label)
		this.type = 'vovis'
		this.repeatable = repeatable
		this.required = required
	}

	createLinkModel() {
		return new LinkModel();
	}

	canLinkToPort(port) {
		if (this.name === 'volume' && port.name.substr(0, 6) === 'volume') {
			return true
		}
		if (port.name === 'volume' && port.name.substr(0, 6) === 'volume') {
			return true
		}
		if (this.name !== port.name) {
			return false
		}
		if (this.in && !this.repeatable && Object.keys(this.getLinks()).length > 1) {
			return false
		}
		if (port.in && !port.repeatable && Object.keys(port.getLinks()).length > 1) {
			return false
		}
		return true
	}
}