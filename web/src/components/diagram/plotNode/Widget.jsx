import * as React from "react";
import { BaseWidget } from "storm-react-diagrams";
import PortLabelWidget from '../port/Widget'
import Scatter from './Scatter'
import Histogram from './Histogram'
import ParallelCordinate from './ParallelCoordinate'

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
        <div style={{ minWidth: '1px', minHeight: '1px' }}>
					{ node.name === 'Scatter plot' && <Scatter ref={node.setElement} model={node} /> }
					{ node.name === 'Histogram' && <Histogram ref={node.setElement} model={node} /> }
					{ node.name === 'Parallel Coordinate' && <ParallelCordinate ref={node.setElement} model={node} /> }
				</div>
			</div>
		);
	}
}