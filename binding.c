#include <assert.h>
#include <bare.h>
#include <js.h>

#include "lib/config.h"
#include "lib/constants.h"
#include "lib/node.h"

static js_value_t *
bare_yoga_exports(js_env_t *env, js_value_t *exports) {
  int err;

  bare_yoga__registry_init(&bare_yoga__nodes);
  bare_yoga__registry_init(&bare_yoga__configs);

#define SIG(...) \
  &((js_callback_signature_t){ \
    .version = 0, \
    .result = js_undefined, \
    .args_len = sizeof((int[]){__VA_ARGS__}) / sizeof(int), \
    .args = (int[]){__VA_ARGS__}, \
  })

#define T(name, fn, signature, typed) \
  { \
    js_value_t *val; \
    err = js_create_typed_function(env, name, -1, fn, signature, typed, NULL, &val); \
    assert(err == 0); \
    err = js_set_named_property(env, exports, name, val); \
    assert(err == 0); \
  }

#define V(name, fn) \
  { \
    js_value_t *val; \
    err = js_create_function(env, name, -1, fn, NULL, &val); \
    assert(err == 0); \
    err = js_set_named_property(env, exports, name, val); \
    assert(err == 0); \
  }

#define ACTION(name, fn) T(name, fn, SIG(js_object, js_uint32), fn##_typed)

#define ENUM(name, fn) \
  T(name, fn, SIG(js_object, js_uint32, js_int32), fn##_typed)

#define FLOAT(name, fn) \
  T(name, fn, SIG(js_object, js_uint32, js_float64), fn##_typed)

#define DIMENSION(name, fn) \
  T(name, fn, SIG(js_object, js_uint32, js_float64, js_int32), fn##_typed)

#define EDGE(name, fn) \
  T(name, fn, SIG(js_object, js_uint32, js_int32, js_float64, js_int32), fn##_typed)

  V("nodeInit", bare_yoga_node_init)
  V("nodeDestroy", bare_yoga_node_destroy)
  V("nodeMeasure", bare_yoga_node_measure)
  V("nodeSetChildren", bare_yoga_node_set_children)
  V("nodeChildCount", bare_yoga_node_child_count)
  V("nodeIsDirty", bare_yoga_node_is_dirty)
  V("nodeHasNewLayout", bare_yoga_node_has_new_layout)
  V("nodeHadOverflow", bare_yoga_node_had_overflow)
  V("nodeLayoutDirection", bare_yoga_node_layout_direction)
  V("nodeLayoutMany", bare_yoga_node_layout_many)
  V("registrySize", bare_yoga_node_registry_size)
  V("measured", bare_yoga_measured)

  V("nodeReset", bare_yoga_node_reset)
  V("nodeMarkDirty", bare_yoga_node_mark_dirty)
  ACTION("nodeClearNewLayout", bare_yoga_node_clear_new_layout)
  ACTION("nodeRemoveAllChildren", bare_yoga_node_remove_all_children)

  V("nodeInsertChild", bare_yoga_node_insert_child)
  T("nodeRemoveChild", bare_yoga_node_remove_child, SIG(js_object, js_uint32, js_uint32), bare_yoga_node_remove_child_typed)
  T("nodeCalculateLayout", bare_yoga_node_calculate_layout, SIG(js_object, js_uint32, js_float64, js_float64, js_int32), bare_yoga_node_calculate_layout_typed)
  T("nodeLayout", bare_yoga_node_layout, SIG(js_object, js_uint32, js_object, js_uint32), bare_yoga_node_layout_typed)

  ENUM("nodeDirection", bare_yoga_node_direction)
  ENUM("nodeFlexDirection", bare_yoga_node_flex_direction)
  ENUM("nodeJustifyContent", bare_yoga_node_justify_content)
  ENUM("nodeAlignContent", bare_yoga_node_align_content)
  ENUM("nodeAlignItems", bare_yoga_node_align_items)
  ENUM("nodeAlignSelf", bare_yoga_node_align_self)
  ENUM("nodePositionType", bare_yoga_node_position_type)
  ENUM("nodeFlexWrap", bare_yoga_node_flex_wrap)
  ENUM("nodeOverflow", bare_yoga_node_overflow)
  ENUM("nodeDisplay", bare_yoga_node_display)

  FLOAT("nodeFlex", bare_yoga_node_flex)
  FLOAT("nodeFlexGrow", bare_yoga_node_flex_grow)
  FLOAT("nodeFlexShrink", bare_yoga_node_flex_shrink)
  FLOAT("nodeAspectRatio", bare_yoga_node_aspect_ratio)

  DIMENSION("nodeWidth", bare_yoga_node_width)
  DIMENSION("nodeHeight", bare_yoga_node_height)
  DIMENSION("nodeFlexBasis", bare_yoga_node_flex_basis)
  DIMENSION("nodeMinWidth", bare_yoga_node_min_width)
  DIMENSION("nodeMinHeight", bare_yoga_node_min_height)
  DIMENSION("nodeMaxWidth", bare_yoga_node_max_width)
  DIMENSION("nodeMaxHeight", bare_yoga_node_max_height)

  EDGE("nodeMargin", bare_yoga_node_margin)
  EDGE("nodePadding", bare_yoga_node_padding)
  EDGE("nodePosition", bare_yoga_node_position)
  EDGE("nodeGap", bare_yoga_node_gap)

  T("nodeBorder", bare_yoga_node_border, SIG(js_object, js_uint32, js_int32, js_float64), bare_yoga_node_border_typed)

  V("configInit", bare_yoga_config_init)
  V("configDestroy", bare_yoga_config_destroy)
  V("configPointScaleFactor", bare_yoga_config_point_scale_factor)
  V("configUseWebDefaults", bare_yoga_config_use_web_defaults)
  V("configErrata", bare_yoga_config_errata)
#undef EDGE
#undef DIMENSION
#undef FLOAT
#undef ENUM
#undef ACTION
#undef V
#undef T
#undef SIG

  return bare_yoga_constants(env, exports);
}

BARE_MODULE(bare_yoga, bare_yoga_exports)
