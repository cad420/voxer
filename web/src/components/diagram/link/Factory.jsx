import React from 'react'
import { DefaultLinkFactory } from 'storm-react-diagrams'
import Model from './Model'

export default class Factory extends DefaultLinkFactory {
	constructor() {
		super();
		this.type = "advanced";
	}

	getNewInstance(initialConfig) {
		return new Model();
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