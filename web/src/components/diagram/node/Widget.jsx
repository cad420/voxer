import * as React from "react";
import { BaseWidget } from "storm-react-diagrams";
import PortLabelWidget from '../port/Widget'

export default class Widget extends BaseWidget {
	constructor(props) {
		super("srd-default-node", props);
		this.state = {};
	}

	generatePort(port) {
		return <PortLabelWidget model={port} key={port.id} />;
	}

	render() {
		return (
			<div {...this.getProps()} style={{ background: this.props.node.color }}>
        <div className={this.bem("__ports")}>
					<div className={this.bem("__in")}>
						{this.props.node.getInPorts().map(this.generatePort.bind(this))}
					</div>
				</div>
				<div className={this.bem("__title")}>
					<br/>
					<div className={this.bem("__name")}>{this.props.node.name}</div>
					<br/>
				</div>
				<div className={this.bem("__ports")}>
					<div className={this.bem("__out")}>
						{this.props.node.getOutPorts().map(this.generatePort.bind(this))}
					</div>
				</div>
			</div>
		);
	}
}