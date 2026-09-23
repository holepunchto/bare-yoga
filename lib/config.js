const binding = require('../binding')

module.exports = exports = class YogaConfig {
  constructor(opts = {}) {
    const { pointScaleFactor = null, useWebDefaults = null, errata = null } = opts

    this._tag = binding.configInit()

    if (pointScaleFactor !== null) this.pointScaleFactor = pointScaleFactor
    if (useWebDefaults !== null) this.useWebDefaults = useWebDefaults
    if (errata !== null) this.errata = errata
  }

  get tag() {
    return this._tag
  }

  set pointScaleFactor(value) {
    binding.configPointScaleFactor(this._tag, value)
  }

  set useWebDefaults(value) {
    binding.configUseWebDefaults(this._tag, value)
  }

  set errata(value) {
    binding.configErrata(this._tag, value)
  }

  destroy() {
    if (this._tag === 0) return

    binding.configDestroy(this._tag)

    this._tag = 0
  }

  [Symbol.dispose]() {
    this.destroy()
  }

  [Symbol.for('bare.inspect')]() {
    return {
      __proto__: { constructor: YogaConfig },
      tag: this._tag
    }
  }
}
