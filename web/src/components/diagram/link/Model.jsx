import { PointModel, DefaultLinkModel } from 'storm-react-diagrams'
import * as _ from 'lodash'

export default class AdvancedLinkModel extends DefaultLinkModel {
	constructor() {
		super("advanced");
		this.setColor('#ddd');
	}

	deSerialize(ob, engine) {
		super.deSerialize(ob, engine);
		this.extras = ob.extras;		
		this.points = _.map(ob.points || [], point => {
			var p = new PointModel(this, { x: point.x, y: point.y });
			p.deSerialize(point, engine);
			return p;
		});

		//deserialize labels
		_.forEach(ob.labels || [], label => {
			let labelOb = engine.getLabelFactory(label.type).getNewInstance();
			labelOb.deSerialize(label, engine);
			this.addLabel(labelOb);
		});

		if (ob.target) {
			this.setTargetPort(
				this.getParent()
					.getNode(ob.target)
					.getPortFromID(ob.targetPort)
			);
		}

		if (ob.source) {
			this.setSourcePort(
				this.getParent()
					.getNode(ob.source)
					.getPortFromID(ob.sourcePort)
			);
		}

		if (ob.target && ob.source) {
			if (ob.sourcePort.in === undefined) {}
			const source = this.sourcePort.in ? this.targetPort : this.sourcePort
			const target = this.sourcePort.in ? this.sourcePort : this.targetPort
			if (target.parent && source.parent) {
				target.parent.extras.values[source.name] = source.parent.extras.values
			}
		}
	}
}