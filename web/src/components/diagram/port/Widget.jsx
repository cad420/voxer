import * as React from "react";
import { PortWidget, BaseWidget } from "storm-react-diagrams";

export default class PortLabelWidget extends BaseWidget {
	constructor(props) {
		super("srd-default-port", props);
	}

	getClassName() {
		const { model } = this.props
		let cn = model.in ? this.bem("--in") : this.bem("--out")
		if (model.repeatable) {
			cn += 'repeatable '
		}
		if (!model.required) {
			cn += 'nullable '
		}
		return super.getClassName() + cn;
	}

	render() {
		const { model } = this.props
		const port = (
			<PortWidget
				extraProps={{ title: model.in ? model.accepts.join(', ') : model.type }}
				node={model.getParent()}
				name={model.name}
			/>
		);
		const label = <div className="name" title={model.in ? model.accepts.join(', ') : model.type}>{model.name}</div>;

		return (
			<div {...this.getProps()}>
        {model.in ? port : label}
        {model.in ? label : port}
			</div>
		);
	}
}