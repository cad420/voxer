type Handler = (type: string, data?: any) => void;

class Emitter {
  all: Record<string, Array<Handler>>;
  constructor(all?: Record<string, Array<Handler>>) {
    this.all = all || Object.create(null);
  }

  on(type: string, handler: Handler) {
    (this.all[type] || (this.all[type] = [])).push(handler);
  }

  off(type?: string, handler?: Handler) {
    if (!type && !handler) {
      this.all = Object.create(null);
    } else if (type && this.all[type] && handler) {
      /* eslint-disable no-bitwise */
      this.all[type].splice(this.all[type].indexOf(handler) >>> 0, 1);
    }
  }

  emit(type: string, evt: any) {
    (this.all[type] || []).map((handler) => handler(evt));
    (this.all["*"] || []).map((handler) => handler(type, evt));
  }
}

export default Emitter;
