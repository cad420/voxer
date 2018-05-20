import { DiagramModel } from 'storm-react-diagrams'

export default class VovisDiagramModel extends DiagramModel {
  addLink(link) {
    link.addListener({
      entityRemoved: () => {
        const source = link.sourcePort.in ? link.targetPort : link.sourcePort
        const target = link.sourcePort.in ? link.sourcePort : link.targetPort
        if (source && target && target.parent.extras && target.parent.extras.values) {
          if (target.parent.extras.children[source.name]) {
            target.parent.extras.values[source.name] = undefined
            target.parent.extras.children[source.name] = false
            target.parent.extras.status = false
          } 
        }
        this.removeLink(link);
      }
    });
    this.links[link.getID()] = link;
    this.iterateListeners((listener, event) => {
      if (listener.linksUpdated) {
        listener.linksUpdated({ ...event, link: link, isCreated: true });
      }
    });
    return link;
  }
}