const binding = require('../binding')

const constants = binding.constants
const UNIT = constants.UNIT

const measured = new Float64Array(binding.measured())

const scratch = new Float64Array(4)

function unitOf(value) {
  if (value === 'auto') return UNIT.AUTO
  if (typeof value === 'string' && value.endsWith('%')) return UNIT.PERCENT
  return UNIT.UNDEFINED
}

function amountOf(value) {
  return typeof value === 'string' ? parseFloat(value) : NaN
}

module.exports = exports = class YogaNode {
  constructor(opts = {}) {
    const { config = null } = opts

    this._tag = binding.nodeInit(config === null ? 0 : config.tag)
    this._measure = null
  }

  get tag() {
    return this._tag
  }

  destroy() {
    if (this._tag === 0) return

    binding.nodeDestroy(this._tag)

    this._tag = 0
    this._measure = null
  }

  set direction(value) {
    binding.nodeDirection(this._tag, value)
  }

  set flexDirection(value) {
    binding.nodeFlexDirection(this._tag, value)
  }

  set justifyContent(value) {
    binding.nodeJustifyContent(this._tag, value)
  }

  set alignContent(value) {
    binding.nodeAlignContent(this._tag, value)
  }

  set alignItems(value) {
    binding.nodeAlignItems(this._tag, value)
  }

  set alignSelf(value) {
    binding.nodeAlignSelf(this._tag, value)
  }

  set positionType(value) {
    binding.nodePositionType(this._tag, value)
  }

  set flexWrap(value) {
    binding.nodeFlexWrap(this._tag, value)
  }

  set overflow(value) {
    binding.nodeOverflow(this._tag, value)
  }

  set display(value) {
    binding.nodeDisplay(this._tag, value)
  }

  set flex(value) {
    binding.nodeFlex(this._tag, value)
  }

  set flexGrow(value) {
    binding.nodeFlexGrow(this._tag, value)
  }

  set flexShrink(value) {
    binding.nodeFlexShrink(this._tag, value)
  }

  set aspectRatio(value) {
    binding.nodeAspectRatio(this._tag, value)
  }

  set width(value) {
    if (typeof value === 'number') binding.nodeWidth(this._tag, value, UNIT.POINT)
    else binding.nodeWidth(this._tag, amountOf(value), unitOf(value))
  }

  set height(value) {
    if (typeof value === 'number') binding.nodeHeight(this._tag, value, UNIT.POINT)
    else binding.nodeHeight(this._tag, amountOf(value), unitOf(value))
  }

  set flexBasis(value) {
    if (typeof value === 'number') binding.nodeFlexBasis(this._tag, value, UNIT.POINT)
    else binding.nodeFlexBasis(this._tag, amountOf(value), unitOf(value))
  }

  set minWidth(value) {
    if (typeof value === 'number') binding.nodeMinWidth(this._tag, value, UNIT.POINT)
    else binding.nodeMinWidth(this._tag, amountOf(value), unitOf(value))
  }

  set minHeight(value) {
    if (typeof value === 'number') binding.nodeMinHeight(this._tag, value, UNIT.POINT)
    else binding.nodeMinHeight(this._tag, amountOf(value), unitOf(value))
  }

  set maxWidth(value) {
    if (typeof value === 'number') binding.nodeMaxWidth(this._tag, value, UNIT.POINT)
    else binding.nodeMaxWidth(this._tag, amountOf(value), unitOf(value))
  }

  set maxHeight(value) {
    if (typeof value === 'number') binding.nodeMaxHeight(this._tag, value, UNIT.POINT)
    else binding.nodeMaxHeight(this._tag, amountOf(value), unitOf(value))
  }

  setMargin(edge, value) {
    if (typeof value === 'number') binding.nodeMargin(this._tag, edge, value, UNIT.POINT)
    else binding.nodeMargin(this._tag, edge, amountOf(value), unitOf(value))
  }

  setPadding(edge, value) {
    if (typeof value === 'number') binding.nodePadding(this._tag, edge, value, UNIT.POINT)
    else binding.nodePadding(this._tag, edge, amountOf(value), unitOf(value))
  }

  setPosition(edge, value) {
    if (typeof value === 'number') binding.nodePosition(this._tag, edge, value, UNIT.POINT)
    else binding.nodePosition(this._tag, edge, amountOf(value), unitOf(value))
  }

  setBorder(edge, value) {
    binding.nodeBorder(this._tag, edge, value)
  }

  setGap(gutter, value) {
    if (typeof value === 'number') binding.nodeGap(this._tag, gutter, value, UNIT.POINT)
    else binding.nodeGap(this._tag, gutter, amountOf(value), unitOf(value))
  }

  insertChild(child, index = this.childCount) {
    binding.nodeInsertChild(this._tag, child._tag, index)
  }

  removeChild(child) {
    binding.nodeRemoveChild(this._tag, child._tag)
  }

  removeAllChildren() {
    binding.nodeRemoveAllChildren(this._tag)
  }

  setChildren(children) {
    if (children instanceof Uint32Array) {
      binding.nodeSetChildren(this._tag, children.buffer, children.byteOffset, children.length)

      return
    }

    const tags = new Uint32Array(children.length)

    for (let i = 0; i < children.length; i++) tags[i] = children[i]._tag

    binding.nodeSetChildren(this._tag, tags.buffer, tags.byteOffset, tags.length)
  }

  get childCount() {
    return binding.nodeChildCount(this._tag)
  }

  get isDirty() {
    return binding.nodeIsDirty(this._tag)
  }

  get hasNewLayout() {
    return binding.nodeHasNewLayout(this._tag)
  }

  get hadOverflow() {
    return binding.nodeHadOverflow(this._tag)
  }

  get layoutDirection() {
    return binding.nodeLayoutDirection(this._tag)
  }

  markDirty() {
    binding.nodeMarkDirty(this._tag)
  }

  clearNewLayout() {
    binding.nodeClearNewLayout(this._tag)
  }

  reset() {
    binding.nodeReset(this._tag)
  }

  set measure(fn) {
    this._measure = fn

    if (fn === null) {
      binding.nodeMeasure(this._tag, null)

      return
    }

    binding.nodeMeasure(this._tag, function onmeasure(width, widthMode, height, heightMode) {
      const size = fn(width, widthMode, height, heightMode)

      measured[0] = size.width
      measured[1] = size.height
    })
  }

  get measure() {
    return this._measure
  }

  calculateLayout(width = NaN, height = NaN, direction = constants.DIRECTION.INHERIT) {
    binding.nodeCalculateLayout(this._tag, width, height, direction)
  }

  get layout() {
    binding.nodeLayout(this._tag, scratch.buffer, 0)

    return {
      left: scratch[0],
      top: scratch[1],
      width: scratch[2],
      height: scratch[3]
    }
  }

  readLayout(out, offset = 0) {
    binding.nodeLayout(this._tag, out.buffer, out.byteOffset + offset * out.BYTES_PER_ELEMENT)
  }

  static readLayouts(tags, out, count = tags.length) {
    binding.nodeLayoutMany(tags.buffer, tags.byteOffset, count, out.buffer, out.byteOffset)
  }

  static get registrySize() {
    return binding.registrySize()
  }

  [Symbol.dispose]() {
    this.destroy()
  }

  [Symbol.for('bare.inspect')]() {
    return {
      __proto__: { constructor: YogaNode },
      tag: this._tag
    }
  }
}

for (const [name, group] of Object.entries(constants)) {
  exports[name] = group
}
