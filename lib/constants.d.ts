/** Yoga's enums, named after the C enums without the `YG` prefix. */
interface YogaConstants {
  UNIT: { UNDEFINED: 0; POINT: 1; PERCENT: 2; AUTO: 3 }
  DIRECTION: { INHERIT: 0; LTR: 1; RTL: 2 }
  FLEX_DIRECTION: { COLUMN: 0; COLUMN_REVERSE: 1; ROW: 2; ROW_REVERSE: 3 }
  JUSTIFY: {
    FLEX_START: 0
    CENTER: 1
    FLEX_END: 2
    SPACE_BETWEEN: 3
    SPACE_AROUND: 4
    SPACE_EVENLY: 5
  }
  ALIGN: {
    AUTO: 0
    FLEX_START: 1
    CENTER: 2
    FLEX_END: 3
    STRETCH: 4
    BASELINE: 5
    SPACE_BETWEEN: 6
    SPACE_AROUND: 7
    SPACE_EVENLY: 8
  }
  POSITION_TYPE: { STATIC: 0; RELATIVE: 1; ABSOLUTE: 2 }
  WRAP: { NO_WRAP: 0; WRAP: 1; WRAP_REVERSE: 2 }
  OVERFLOW: { VISIBLE: 0; HIDDEN: 1; SCROLL: 2 }
  DISPLAY: { FLEX: 0; NONE: 1 }
  EDGE: {
    LEFT: 0
    TOP: 1
    RIGHT: 2
    BOTTOM: 3
    START: 4
    END: 5
    HORIZONTAL: 6
    VERTICAL: 7
    ALL: 8
  }
  GUTTER: { COLUMN: 0; ROW: 1; ALL: 2 }
  MEASURE_MODE: { UNDEFINED: 0; EXACTLY: 1; AT_MOST: 2 }
  ERRATA: { NONE: 0; STRETCH_FLEX_BASIS: 1; ALL: 2147483647; CLASSIC: 2147483646 }
}

type ValueOf<T> = T[keyof T]

declare namespace YogaConstants {
  export type Direction = ValueOf<YogaConstants['DIRECTION']>
  export type FlexDirection = ValueOf<YogaConstants['FLEX_DIRECTION']>
  export type Justify = ValueOf<YogaConstants['JUSTIFY']>
  export type Align = ValueOf<YogaConstants['ALIGN']>
  export type PositionType = ValueOf<YogaConstants['POSITION_TYPE']>
  export type Wrap = ValueOf<YogaConstants['WRAP']>
  export type Overflow = ValueOf<YogaConstants['OVERFLOW']>
  export type Display = ValueOf<YogaConstants['DISPLAY']>
  export type Edge = ValueOf<YogaConstants['EDGE']>
  export type Gutter = ValueOf<YogaConstants['GUTTER']>
  export type MeasureMode = ValueOf<YogaConstants['MEASURE_MODE']>
  /** A combination of `ERRATA` flags. */
  export type Errata = number
}

export = YogaConstants
