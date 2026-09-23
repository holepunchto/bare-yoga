#pragma once

#include <assert.h>
#include <js.h>

#include <yoga/Yoga.h>

static js_value_t *
bare_yoga_constants(js_env_t *env, js_value_t *exports) {
  int err;

  js_value_t *constants;
  err = js_create_object(env, &constants);
  assert(err == 0);

#define V(group, name, value) \
  { \
    js_value_t *val; \
    err = js_create_int32(env, value, &val); \
    assert(err == 0); \
    err = js_set_named_property(env, group, name, val); \
    assert(err == 0); \
  }

#define G(name, block) \
  { \
    js_value_t *group; \
    err = js_create_object(env, &group); \
    assert(err == 0); \
    block; \
    err = js_set_named_property(env, constants, name, group); \
    assert(err == 0); \
  }

  G("UNIT", {
    V(group, "UNDEFINED", YGUnitUndefined);
    V(group, "POINT", YGUnitPoint);
    V(group, "PERCENT", YGUnitPercent);
    V(group, "AUTO", YGUnitAuto);
  })

  G("DIRECTION", {
    V(group, "INHERIT", YGDirectionInherit);
    V(group, "LTR", YGDirectionLTR);
    V(group, "RTL", YGDirectionRTL);
  })

  G("FLEX_DIRECTION", {
    V(group, "COLUMN", YGFlexDirectionColumn);
    V(group, "COLUMN_REVERSE", YGFlexDirectionColumnReverse);
    V(group, "ROW", YGFlexDirectionRow);
    V(group, "ROW_REVERSE", YGFlexDirectionRowReverse);
  })

  G("JUSTIFY", {
    V(group, "FLEX_START", YGJustifyFlexStart);
    V(group, "CENTER", YGJustifyCenter);
    V(group, "FLEX_END", YGJustifyFlexEnd);
    V(group, "SPACE_BETWEEN", YGJustifySpaceBetween);
    V(group, "SPACE_AROUND", YGJustifySpaceAround);
    V(group, "SPACE_EVENLY", YGJustifySpaceEvenly);
  })

  G("ALIGN", {
    V(group, "AUTO", YGAlignAuto);
    V(group, "FLEX_START", YGAlignFlexStart);
    V(group, "CENTER", YGAlignCenter);
    V(group, "FLEX_END", YGAlignFlexEnd);
    V(group, "STRETCH", YGAlignStretch);
    V(group, "BASELINE", YGAlignBaseline);
    V(group, "SPACE_BETWEEN", YGAlignSpaceBetween);
    V(group, "SPACE_AROUND", YGAlignSpaceAround);
    V(group, "SPACE_EVENLY", YGAlignSpaceEvenly);
  })

  G("POSITION_TYPE", {
    V(group, "STATIC", YGPositionTypeStatic);
    V(group, "RELATIVE", YGPositionTypeRelative);
    V(group, "ABSOLUTE", YGPositionTypeAbsolute);
  })

  G("WRAP", {
    V(group, "NO_WRAP", YGWrapNoWrap);
    V(group, "WRAP", YGWrapWrap);
    V(group, "WRAP_REVERSE", YGWrapWrapReverse);
  })

  G("OVERFLOW", {
    V(group, "VISIBLE", YGOverflowVisible);
    V(group, "HIDDEN", YGOverflowHidden);
    V(group, "SCROLL", YGOverflowScroll);
  })

  G("DISPLAY", {
    V(group, "FLEX", YGDisplayFlex);
    V(group, "NONE", YGDisplayNone);
  })

  G("EDGE", {
    V(group, "LEFT", YGEdgeLeft);
    V(group, "TOP", YGEdgeTop);
    V(group, "RIGHT", YGEdgeRight);
    V(group, "BOTTOM", YGEdgeBottom);
    V(group, "START", YGEdgeStart);
    V(group, "END", YGEdgeEnd);
    V(group, "HORIZONTAL", YGEdgeHorizontal);
    V(group, "VERTICAL", YGEdgeVertical);
    V(group, "ALL", YGEdgeAll);
  })

  G("GUTTER", {
    V(group, "COLUMN", YGGutterColumn);
    V(group, "ROW", YGGutterRow);
    V(group, "ALL", YGGutterAll);
  })

  G("MEASURE_MODE", {
    V(group, "UNDEFINED", YGMeasureModeUndefined);
    V(group, "EXACTLY", YGMeasureModeExactly);
    V(group, "AT_MOST", YGMeasureModeAtMost);
  })

  G("ERRATA", {
    V(group, "NONE", YGErrataNone);
    V(group, "STRETCH_FLEX_BASIS", YGErrataStretchFlexBasis);
    V(group, "ALL", YGErrataAll);
    V(group, "CLASSIC", YGErrataClassic);
  })
#undef G
#undef V

  err = js_set_named_property(env, exports, "constants", constants);
  assert(err == 0);

  return exports;
}
