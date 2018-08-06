import { DefaultPortModel } from 'storm-react-diagrams'
import LinkModel from '../link/Model'

export default class Model extends DefaultPortModel {
	constructor(isInput, name, type, repeatable, required = true) {
		super(isInput, name, type)
		if (isInput) this.accepts = type;
		else this.type = type;
		this.repeatable = repeatable;
		this.required = required;
	}

	createLinkModel() {
		return new LinkModel();
	}

	canLinkToPort(port) {
		if (this.in) {
			if (this.accepts.indexOf(port.type) === -1) {
				return false;
			}
			if (this.repeatable && Object.keys(this.getLinks()).length > 1) {
				return false;
			}
			return true;
		} else if (port.in) {
			if (port.accepts.indexOf(this.type) === -1) {
				return false;
			}
			if (!port.repeatable && Object.keys(port.getLinks()).length > 1) {
				return false
			}
			return true;
		}
		return false;
	}
}