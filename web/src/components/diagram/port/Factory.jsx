import Model from './Model'
import { AbstractPortFactory } from 'storm-react-diagrams'

export default class VovisPortFactory extends AbstractPortFactory {
	constructor() {
		super("vovis");
		this.type = 'vovis'
	}

	getNewInstance(initialConfig) {
		return new Model(true, "unknown");
	}
}