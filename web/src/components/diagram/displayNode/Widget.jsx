import * as React from "react";
import { BaseWidget } from "storm-react-diagrams";
import PortLabelWidget from '../port/Widget'
import Display from '../../Display' 
import Animate from '../../Animation'

export default class Widget extends BaseWidget {
	constructor(props) {
		super("srd-default-node", props);
		this.state = {
      ws: null
    };
	}

	generatePort(port) {
		return <PortLabelWidget model={port} key={port.id} />;
  }
  
	render() {
		const { node } = this.props
		return (
			<div {...this.getProps()} style={{ background: node.color }}>
        <div className={this.bem("__ports")}>
					<div className={this.bem("__in")}>
						{node.getInPorts().map(this.generatePort.bind(this))}
					</div>
				</div>
        <div className={this.bem("__title")}>
					<br/>
					<div className={this.bem("__name")}>{node.name}</div>
					<br/>
				</div>
        <div style={{ width: '500px', minHeight: '1px' }}>
					{ node.name === 'Image Display' && <Display ref={node.setElement} model={node} /> }
					{ node.name === 'Animation Display' && <Animate ref={node.setElement} model={node} /> }
				</div>
			</div>
		);
	}
}