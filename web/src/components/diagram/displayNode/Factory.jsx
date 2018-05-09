import { AbstractNodeFactory } from "storm-react-diagrams";
import Model from './Model'
import Widget from "./Widget";
import * as React from "react";

export default class Factory extends AbstractNodeFactory {
	constructor() {
		super("display");
	}

	generateReactWidget(diagramEngine, node) {
		return <Widget node={node} />;
	}

	getNewInstance() {
		return new Model();
	}
}