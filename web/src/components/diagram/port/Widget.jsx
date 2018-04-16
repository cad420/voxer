import * as React from "react";
import { PortWidget, BaseWidget } from "storm-react-diagrams";

export default class PortLabelWidget extends BaseWidget {
	constructor(props) {
		super("srd-default-port", props);
	}

	getClassName() {
		return super.getClassName() + (this.props.model.in ? this.bem("--in") : this.bem("--out"));
	}

	render() {
		const { model } = this.props
		var port = <PortWidget node={model.getParent()} name={model.name} />;
		var label = <div className="name" title={model.name}>{model.label}</div>;

		return (
			<div {...this.getProps()}>
        {port}
        {label}
			</div>
		);
	}
}