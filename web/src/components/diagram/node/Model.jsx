import { NodeModel } from "storm-react-diagrams";
import PortModel from '../port/Model'

import * as _ from "lodash";

export default class Model extends NodeModel {
	constructor(name, color) {
    super("vovis");
    this.name = name;
    this.color = color;
    this.ports = {};
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

	addOutPort(name, label) {
		return this.addPort(new PortModel(false, name, label));
	}

	getInPorts() {
		return _.filter(this.ports, portModel => {
			return portModel.in;
		});
	}

	getOutPorts() {
		return _.filter(this.ports, portModel => {
			return !portModel.in;
		});
	}
}