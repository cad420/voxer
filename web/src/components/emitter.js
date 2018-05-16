export default class {
  constructor(all) {
    this.all = all || Object.create(null)
  }

  on(type, handler) {
    (this.all[type] || (this.all[type] = [])).push(handler)
  }

  off(type, handler) {
    if (!type && !handler) {
      this.all = Object.create(null)
    } else if (this.all[type]) {
      this.all[type].splice(this.all[type].indexOf(handler) >>> 0, 1)
    }
  }

  emit(type, evt) {
    (this.all[type] || []).map((handler) => handler(evt))
      ; (this.all['*'] || []).map((handler) => handler(type, evt))
  }
}