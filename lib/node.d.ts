import YogaConfig = require('./config')
import YogaConstants = require('./constants')

type Edge = YogaConstants.Edge

/** A length in points, a percentage of the parent such as `'50%'`, or `'auto'`. */
type Value = number | `${number}%` | 'auto'

/** The position and size of a node relative to its parent, in points. */
interface Layout {
  left: number
  top: number
  width: number
  height: number
}

/**
 * Called when Yoga needs the size of a leaf node, such as a run of text. Each dimension comes with
 * a `MEASURE_MODE` that says whether it is undefined, exact or a maximum.
 */
type MeasureFunction = (
  width: number,
  widthMode: YogaConstants.MeasureMode,
  height: number,
  heightMode: YogaConstants.MeasureMode
) => { width: number; height: number }

/**
 * A node in a layout tree. The style properties are write-only, and the computed layout is read
 * after `calculateLayout()`.
 */
interface YogaNode extends Disposable {
  /** The tag that identifies the node to the binding. Zero once destroyed. */
  readonly tag: number

  set direction(value: YogaConstants.Direction)
  set flexDirection(value: YogaConstants.FlexDirection)
  set justifyContent(value: YogaConstants.Justify)
  set alignContent(value: YogaConstants.Align)
  set alignItems(value: YogaConstants.Align)
  set alignSelf(value: YogaConstants.Align)
  set positionType(value: YogaConstants.PositionType)
  set flexWrap(value: YogaConstants.Wrap)
  set overflow(value: YogaConstants.Overflow)
  set display(value: YogaConstants.Display)
  set flex(value: number)
  set flexGrow(value: number)
  set flexShrink(value: number)
  set aspectRatio(value: number)
  set width(value: Value)
  set height(value: Value)
  set flexBasis(value: Value)
  set minWidth(value: Value)
  set minHeight(value: Value)
  set maxWidth(value: Value)
  set maxHeight(value: Value)

  setMargin(edge: Edge, value: Value): void
  setPadding(edge: Edge, value: Value): void
  setPosition(edge: Edge, value: Value): void
  setBorder(edge: Edge, value: number): void
  setGap(gutter: YogaConstants.Gutter, value: Value): void

  /** Insert `child` at `index`, which defaults to the end. A node can only have one parent. */
  insertChild(child: YogaNode, index?: number): void
  removeChild(child: YogaNode): void
  removeAllChildren(): void
  /** Replace the children of the node, given as nodes or as their tags. */
  setChildren(children: YogaNode[] | Uint32Array): void

  readonly childCount: number
  readonly isDirty: boolean
  /** Whether the layout changed in the last `calculateLayout()`. Cleared by `clearNewLayout()`. */
  readonly hasNewLayout: boolean
  /** Whether the children overflowed the node in the last `calculateLayout()`. */
  readonly hadOverflow: boolean
  /** The direction the node was laid out in, as a `DIRECTION`. */
  readonly layoutDirection: YogaConstants.Direction
  /** The position of the node from the right of its parent. Only set along a reversed row. */
  readonly layoutRight: number
  /** The position of the node from the bottom of its parent. Only set along a reversed column. */
  readonly layoutBottom: number

  /** The resolved margin at `edge`, which must be a single edge, not `HORIZONTAL`, `VERTICAL` or `ALL`. */
  layoutMargin(edge: Edge): number
  /** The resolved border at `edge`, which must be a single edge, not `HORIZONTAL`, `VERTICAL` or `ALL`. */
  layoutBorder(edge: Edge): number
  /** The resolved padding at `edge`, which must be a single edge, not `HORIZONTAL`, `VERTICAL` or `ALL`. */
  layoutPadding(edge: Edge): number

  /** Mark a node with a measure function as needing to be measured again. */
  markDirty(): void
  clearNewLayout(): void
  /** Reset the node to its defaults. It must have no parent and no children. */
  reset(): void

  /** The function that measures the node, or `null`. A node with one cannot have children. */
  measure: MeasureFunction | null

  /**
   * Lay out the tree from this node within `width` by `height` points. Both default to `NaN`, which
   * leaves the dimension unbounded. `direction` defaults to `DIRECTION.INHERIT`.
   */
  calculateLayout(width?: number, height?: number, direction?: YogaConstants.Direction): void

  /** The computed position and size of the node. */
  readonly layout: Layout

  /** Write the computed `left`, `top`, `width` and `height` to `out`, starting at `offset`. */
  readLayout(out: Float64Array, offset?: number): void

  /** Free the node. Its children are left without a parent. */
  destroy(): void
}

declare class YogaNode {
  constructor(opts?: { config?: YogaConfig | null })

  /**
   * Read the layout of the first `count` nodes in `tags` into `out`, five values per node: whether
   * it has a new layout, as `1` or `0`, then `left`, `top`, `width` and `height`.
   */
  static readLayouts(tags: Uint32Array, out: Float64Array, count?: number): void

  /** How many nodes are alive. */
  static readonly registrySize: number

  static readonly UNIT: YogaConstants['UNIT']
  static readonly DIRECTION: YogaConstants['DIRECTION']
  static readonly FLEX_DIRECTION: YogaConstants['FLEX_DIRECTION']
  static readonly JUSTIFY: YogaConstants['JUSTIFY']
  static readonly ALIGN: YogaConstants['ALIGN']
  static readonly POSITION_TYPE: YogaConstants['POSITION_TYPE']
  static readonly WRAP: YogaConstants['WRAP']
  static readonly OVERFLOW: YogaConstants['OVERFLOW']
  static readonly DISPLAY: YogaConstants['DISPLAY']
  static readonly EDGE: YogaConstants['EDGE']
  static readonly GUTTER: YogaConstants['GUTTER']
  static readonly MEASURE_MODE: YogaConstants['MEASURE_MODE']
  static readonly ERRATA: YogaConstants['ERRATA']
}

declare namespace YogaNode {
  export { type Layout, type MeasureFunction, type Value }
}

export = YogaNode
