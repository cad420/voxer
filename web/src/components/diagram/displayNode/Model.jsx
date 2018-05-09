import { NodeModel } from "storm-react-diagrams";
import PortModel from '../port/Model'

import * as _ from "lodash";

export default class Model extends NodeModel {
	constructor(name, color) {
    super("display");
    this.name = name;
    this.color = color;
		this.ports = {};
		this.el = null;
  }

	deSerialize(object, engine) {
		super.deSerialize(object, engine);
		this.name = object.name;
		this.color = object.color;
	}

	serialize() {
		return _.merge(super.serialize(), {
			name: this.name,
			color: this.color
		});
	}

	addInPort(name, label) {
		return this.addPort(new PortModel(true, name, label));
	}

	getInPorts() {
		return _.filter(this.ports, portModel => {
			return portModel.in;
		});
	}

	setElement = el => {
		this.el = el
	}
}