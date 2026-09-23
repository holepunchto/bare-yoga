const test = require('brittle')
const { Node, Config, constants } = require('.')

const { EDGE, FLEX_DIRECTION, JUSTIFY, ALIGN, MEASURE_MODE, GUTTER } = constants

test('lays out a row', (t) => {
  const root = new Node()
  root.width = 300
  root.height = 100
  root.flexDirection = FLEX_DIRECTION.ROW

  const a = new Node()
  a.flexGrow = 1

  const b = new Node()
  b.flexGrow = 2

  root.insertChild(a)
  root.insertChild(b)

  root.calculateLayout(300, 100)

  t.alike(a.layout, { left: 0, top: 0, width: 100, height: 100 })
  t.alike(b.layout, { left: 100, top: 0, width: 200, height: 100 })

  for (const node of [b, a, root]) node.destroy()
})

test('percent and auto dimensions', (t) => {
  const root = new Node()
  root.width = 400
  root.height = 200

  const half = new Node()
  half.width = '50%'
  half.height = '25%'

  root.insertChild(half)
  root.calculateLayout(400, 200)

  t.is(half.layout.width, 200)
  t.is(half.layout.height, 50)

  half.width = 'auto'
  root.calculateLayout(400, 200)

  t.is(half.layout.width, 400, 'auto stretches to the cross axis')

  for (const node of [half, root]) node.destroy()
})

test('margin, padding and border', (t) => {
  const root = new Node()
  root.width = 200
  root.height = 200
  root.setPadding(EDGE.ALL, 10)
  root.setBorder(EDGE.ALL, 5)

  const child = new Node()
  child.flexGrow = 1
  child.setMargin(EDGE.LEFT, 20)

  root.insertChild(child)
  root.calculateLayout(200, 200)

  t.is(child.layout.left, 35, 'border plus padding plus margin')
  t.is(child.layout.top, 15, 'border plus padding')
  t.is(child.layout.width, 150)

  for (const node of [child, root]) node.destroy()
})

test('justify and align', (t) => {
  const root = new Node()
  root.width = 100
  root.height = 100
  root.flexDirection = FLEX_DIRECTION.ROW
  root.justifyContent = JUSTIFY.CENTER
  root.alignItems = ALIGN.CENTER

  const child = new Node()
  child.width = 20
  child.height = 20

  root.insertChild(child)
  root.calculateLayout(100, 100)

  t.alike(child.layout, { left: 40, top: 40, width: 20, height: 20 })

  for (const node of [child, root]) node.destroy()
})

test('gap', (t) => {
  const root = new Node()
  root.width = 100
  root.height = 20
  root.flexDirection = FLEX_DIRECTION.ROW
  root.setGap(GUTTER.COLUMN, 10)

  const a = new Node()
  a.flexGrow = 1

  const b = new Node()
  b.flexGrow = 1

  root.insertChild(a)
  root.insertChild(b)
  root.calculateLayout(100, 20)

  t.is(a.layout.width, 45)
  t.is(b.layout.left, 55)

  for (const node of [b, a, root]) node.destroy()
})

test('measure function', (t) => {
  const root = new Node()
  root.width = 100
  root.height = 100
  // Without this the cross axis stretches the leaf, so its width is imposed
  // rather than measured.
  root.alignItems = ALIGN.FLEX_START

  const leaf = new Node()

  const seen = []

  leaf.measure = (width, widthMode, height, heightMode) => {
    seen.push({ width, widthMode })

    return { width: 30, height: 40 }
  }

  root.insertChild(leaf)
  root.calculateLayout(100, 100)

  t.is(leaf.layout.width, 30)
  t.is(leaf.layout.height, 40)
  t.ok(seen.length > 0, 'measure was called')
  t.is(seen[0].widthMode, MEASURE_MODE.AT_MOST)

  for (const node of [leaf, root]) node.destroy()
})

test('measure reruns after mark dirty', (t) => {
  const root = new Node()
  root.width = 100
  root.height = 100
  root.alignItems = ALIGN.FLEX_START

  let size = { width: 10, height: 10 }

  const leaf = new Node()
  leaf.measure = () => size

  root.insertChild(leaf)
  root.calculateLayout(100, 100)

  t.is(leaf.layout.width, 10)

  size = { width: 50, height: 50 }
  leaf.markDirty()
  root.calculateLayout(100, 100)

  t.is(leaf.layout.width, 50)

  for (const node of [leaf, root]) node.destroy()
})

test('bulk layout read', (t) => {
  const root = new Node()
  root.width = 300
  root.height = 50
  root.flexDirection = FLEX_DIRECTION.ROW

  const a = new Node()
  a.flexGrow = 1

  const b = new Node()
  b.flexGrow = 1

  root.insertChild(a)
  root.insertChild(b)
  root.calculateLayout(300, 50)

  const tags = new Uint32Array([root.tag, a.tag, b.tag])
  const out = new Float64Array(tags.length * 5)

  Node.readLayouts(tags, out)

  t.is(out[0], 1, 'root has new layout')
  t.alike(Array.from(out.subarray(1, 5)), [0, 0, 300, 50])
  t.alike(Array.from(out.subarray(6, 10)), [0, 0, 150, 50])
  t.alike(Array.from(out.subarray(11, 15)), [150, 0, 150, 50])

  for (const node of [root, a, b]) node.clearNewLayout()

  Node.readLayouts(tags, out)

  t.is(out[0], 0, 'cleared')
  t.is(out[5], 0)

  for (const node of [b, a, root]) node.destroy()
})

test('bulk read matches the single read', (t) => {
  const root = new Node()
  root.width = 120
  root.height = 60
  root.flexDirection = FLEX_DIRECTION.ROW

  const children = []

  for (let i = 0; i < 4; i++) {
    const child = new Node()
    child.flexGrow = i + 1
    root.insertChild(child)
    children.push(child)
  }

  root.calculateLayout(120, 60)

  const nodes = [root, ...children]
  const tags = new Uint32Array(nodes.map((node) => node.tag))
  const out = new Float64Array(nodes.length * 5)

  Node.readLayouts(tags, out)

  for (let i = 0; i < nodes.length; i++) {
    const { left, top, width, height } = nodes[i].layout

    t.alike(Array.from(out.subarray(i * 5 + 1, i * 5 + 5)), [left, top, width, height])
  }

  for (const node of [...children, root]) node.destroy()
})

test('setChildren replaces the child list', (t) => {
  const root = new Node()
  root.width = 100
  root.height = 100

  const a = new Node()
  const b = new Node()
  const c = new Node()

  root.setChildren([a, b])
  t.is(root.childCount, 2)

  root.setChildren([c])
  t.is(root.childCount, 1)

  root.setChildren([])
  t.is(root.childCount, 0)

  for (const node of [a, b, c, root]) node.destroy()
})

test('config point scale factor rounds layout', (t) => {
  const config = new Config({ pointScaleFactor: 1 })

  const root = new Node({ config })
  root.width = 100
  root.height = 100

  const child = new Node({ config })
  child.flexGrow = 1
  child.setMargin(EDGE.LEFT, 0.5)

  root.insertChild(child)
  root.calculateLayout(100, 100)

  t.is(child.layout.left % 1, 0, 'rounded to whole points')

  for (const node of [child, root]) node.destroy()

  config.destroy()
})

test('destroy releases the handle', async (t) => {
  const before = Node.registrySize

  const node = new Node()

  t.is(Node.registrySize, before + 1)

  const tag = node.tag

  node.destroy()

  t.is(Node.registrySize, before)

  node.destroy()

  t.is(Node.registrySize, before, 'destroy is idempotent')

  const tags = new Uint32Array([tag])
  const out = new Float64Array(5)

  await t.exception.all(() => Node.readLayouts(tags, out), /Unknown node/)
})

test('using disposes a node and a config', (t) => {
  const before = Node.registrySize

  {
    using config = new Config({ pointScaleFactor: 1 })
    using root = new Node({ config })

    root.width = 100
    root.height = 100

    root.calculateLayout(100, 100)

    t.is(Node.registrySize, before + 1)
    t.not(config.tag, 0)
  }

  t.is(Node.registrySize, before, 'the node was destroyed on scope exit')
})

test('rejects an undersized buffer', async (t) => {
  await t.exception.all(
    () => Node.readLayouts(new Uint32Array(1), new Float64Array(2)),
    /room for 5 doubles/
  )
})

test('refuses a child that already has a parent', async (t) => {
  const a = new Node()
  const b = new Node()
  const c = new Node()

  a.insertChild(c)

  await t.exception(() => b.insertChild(c), /already has a parent/)

  for (const node of [c, a, b]) node.destroy()
})

test('refuses to measure a node with children', async (t) => {
  const root = new Node()
  const child = new Node()

  root.insertChild(child)

  await t.exception(() => {
    root.measure = () => ({ width: 0, height: 0 })
  }, /Only a leaf node can measure/)

  for (const node of [child, root]) node.destroy()
})

test('refuses to add a child to a measured node', async (t) => {
  const root = new Node()
  root.measure = () => ({ width: 0, height: 0 })

  const child = new Node()

  await t.exception(() => root.insertChild(child), /cannot have children/)

  for (const node of [child, root]) node.destroy()
})

test('refuses to mark an unmeasured node dirty', async (t) => {
  const node = new Node()

  await t.exception(() => node.markDirty(), /Only a measured node/)

  node.destroy()
})

test('refuses to reset an attached node', async (t) => {
  const root = new Node()
  const child = new Node()

  root.insertChild(child)

  await t.exception(() => root.reset(), /still has children|with children/)
  await t.exception(() => child.reset(), /with a parent/)

  for (const node of [child, root]) node.destroy()
})

test('every style property is reachable', (t) => {
  const node = new Node()

  const { DIRECTION, FLEX_DIRECTION, POSITION_TYPE, WRAP, OVERFLOW, DISPLAY } = constants

  node.direction = DIRECTION.LTR
  node.flexDirection = FLEX_DIRECTION.ROW_REVERSE
  node.justifyContent = JUSTIFY.SPACE_EVENLY
  node.alignContent = ALIGN.SPACE_AROUND
  node.alignItems = ALIGN.BASELINE
  node.alignSelf = ALIGN.CENTER
  node.positionType = POSITION_TYPE.RELATIVE
  node.flexWrap = WRAP.WRAP_REVERSE
  node.overflow = OVERFLOW.SCROLL
  node.display = DISPLAY.FLEX

  node.flex = 1
  node.flexGrow = 2
  node.flexShrink = 3
  node.aspectRatio = 1.5

  for (const name of [
    'width',
    'height',
    'flexBasis',
    'minWidth',
    'minHeight',
    'maxWidth',
    'maxHeight'
  ]) {
    node[name] = 10
    node[name] = '10%'
    node[name] = 'auto'
    node[name] = undefined
  }

  for (const edge of Object.values(EDGE)) {
    node.setMargin(edge, 1)
    node.setMargin(edge, '1%')
    node.setMargin(edge, 'auto')
    node.setPadding(edge, 1)
    node.setPadding(edge, '1%')
    node.setPosition(edge, 1)
    node.setPosition(edge, '1%')
    node.setBorder(edge, 1)
  }

  for (const gutter of Object.values(GUTTER)) {
    node.setGap(gutter, 1)
    node.setGap(gutter, '1%')
  }

  t.pass('all setters dispatched')

  node.destroy()
})

test('queries and readLayout', (t) => {
  const root = new Node()
  root.width = 40
  root.height = 20

  const child = new Node()
  child.flexGrow = 1
  root.insertChild(child)

  t.is(root.childCount, 1)
  t.is(root.isDirty, true, 'dirty before the first pass')

  root.calculateLayout(40, 20)

  t.is(root.hasNewLayout, true)
  t.is(root.hadOverflow, false)
  t.is(root.layoutDirection, constants.DIRECTION.LTR)
  t.is(root.isDirty, false)

  const out = new Float64Array(8)
  root.readLayout(out, 4)

  t.alike(Array.from(out.subarray(4)), [0, 0, 40, 20])

  root.removeChild(child)
  t.is(root.childCount, 0)

  root.insertChild(child)
  root.removeAllChildren()
  t.is(root.childCount, 0)

  for (const node of [child, root]) node.destroy()
})

test('measure can be read back and cleared', (t) => {
  const node = new Node()

  const fn = () => ({ width: 1, height: 1 })

  node.measure = fn
  t.is(node.measure, fn)

  node.measure = null
  t.is(node.measure, null)

  node.destroy()
})

test('config options', (t) => {
  const config = new Config({
    pointScaleFactor: 2,
    useWebDefaults: false,
    errata: constants.ERRATA.NONE
  })

  t.ok(config.tag > 0)

  const node = new Node({ config })
  node.width = 10
  node.height = 10
  node.calculateLayout(10, 10)

  t.is(node.layout.width, 10)

  node.destroy()

  config.destroy()
  config.destroy()

  t.is(config.tag, 0, 'destroy is idempotent')
})

test('a node can be reset and reused', (t) => {
  const node = new Node()
  node.width = 50
  node.height = 50
  node.calculateLayout(50, 50)

  t.is(node.layout.width, 50)

  node.reset()
  node.width = 25
  node.height = 25
  node.calculateLayout(25, 25)

  t.is(node.layout.width, 25)

  node.destroy()
})

test('setChildren accepts tags directly', (t) => {
  const root = new Node()
  const a = new Node()
  const b = new Node()

  root.setChildren(new Uint32Array([a.tag, b.tag]))

  t.is(root.childCount, 2)

  root.setChildren(new Uint32Array(0))

  t.is(root.childCount, 0)

  for (const node of [a, b, root]) node.destroy()
})

test('inspects as its own class', (t) => {
  const node = new Node()
  const config = new Config()

  const inspect = Symbol.for('bare.inspect')

  t.is(Object.getPrototypeOf(node[inspect]()).constructor.name, 'YogaNode')
  t.is(Object.getPrototypeOf(config[inspect]()).constructor.name, 'YogaConfig')

  node.destroy()
  config.destroy()
})
