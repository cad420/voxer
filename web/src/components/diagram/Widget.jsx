import { DiagramWidget, MoveItemsAction, PointModel, PortModel } from 'storm-react-diagrams'
import _ from 'lodash'

export default class VovisDiagramWidget extends DiagramWidget {
  onMouseUp(event) {
    const diagramEngine = this.props.diagramEngine;
		if (this.state.action instanceof MoveItemsAction) {
			var element = this.getMouseElement(event);
			_.forEach(this.state.action.selectionModels, model => {
				//only care about points connecting to things
				if (!(model.model instanceof PointModel)) {
					return;
				}
				if (element && element.model instanceof PortModel && !diagramEngine.isModelLocked(element.model)) {
					let link = model.model.getLink();
					if (link.getTargetPort() !== null) {
						//if this was a valid link already and we are adding a node in the middle, create 2 links from the original
						if (link.getTargetPort() !== element.model && link.getSourcePort() !== element.model) {
							const targetPort = link.getTargetPort();
							let newLink = link.clone({});
							newLink.setSourcePort(element.model);
							newLink.setTargetPort(targetPort);
							link.setTargetPort(element.model);
							targetPort.removeLink(link);
							newLink.removePointsBefore(newLink.getPoints()[link.getPointIndex(model.model)]);
							link.removePointsAfter(model.model);
							diagramEngine.getDiagramModel().addLink(newLink);
							//if we are connecting to the same target or source, remove tweener points
						} else if (link.getTargetPort() === element.model) {
							link.removePointsAfter(model.model);
						} else if (link.getSourcePort() === element.model) {
							link.removePointsBefore(model.model);
						}
					} else {
						link.setTargetPort(element.model);
					}
					delete this.props.diagramEngine.linksThatHaveInitiallyRendered[link.getID()];
				}
			});

			//check for / remove any loose links in any models which have been moved
			if (!this.props.allowLooseLinks && this.state.wasMoved) {
				_.forEach(this.state.action.selectionModels, model => {
					//only care about points connecting to things
					if (!(model.model instanceof PointModel)) {
						return;
					}

					let selectedPoint = model.model;
					let link = selectedPoint.getLink();
					if (link.getSourcePort() === null || link.getTargetPort() === null) {
						link.remove();
					}
				});
			}

			//remove any invalid links
			_.forEach(this.state.action.selectionModels, model => {
				//only care about points connecting to things
				if (!(model.model instanceof PointModel)) {
					return;
				}

				let link = model.model.getLink();
				let sourcePort = link.getSourcePort();
				let targetPort = link.getTargetPort();
				if (sourcePort !== null && targetPort !== null) {
					if (!sourcePort.canLinkToPort(targetPort)) {
						//link not allowed
						link.remove();
					} else if (
						_.some(
							_.values(targetPort.getLinks()),
							(l) =>
								l !== link && (l.getSourcePort() === sourcePort || l.getTargetPort() === sourcePort)
						)
					) {
						//link is a duplicate
						link.remove();
					} else {
            const source = sourcePort.in ? targetPort : sourcePort
            const target = sourcePort.in ? sourcePort : targetPort
						target.parent.extras.values[source.name] = source.parent.extras.values
						if (target.parent.type === 'display') {
							target.parent.el.update()
						}
          }
				}
			});

			diagramEngine.clearRepaintEntities();
			this.stopFiringAction(!this.state.wasMoved);
		} else {
			diagramEngine.clearRepaintEntities();
			this.stopFiringAction();
		}
		this.state.document.removeEventListener("mousemove", this.onMouseMove);
		this.state.document.removeEventListener("mouseup", this.onMouseUp);
  }
}