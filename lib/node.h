#pragma once

#include <assert.h>
#include <js.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <yoga/Yoga.h>

#include "bridging.h"
#include "config.h"
#include "registry.h"

static bare_yoga_registry_t bare_yoga__nodes;

typedef struct {
  js_env_t *env;
  js_ref_t *measure;
} bare_yoga_node_t;

static inline YGNodeRef
bare_yoga__node(uint32_t tag) {
  return (YGNodeRef) bare_yoga__registry_get(&bare_yoga__nodes, tag);
}

static bool
bare_yoga__check(js_env_t *env, bool condition, const char *message) {
  if (condition) return true;

  int err = js_throw_error(env, NULL, message);
  assert(err == 0);

  return false;
}

static double bare_yoga__measured[2];

static YGSize
bare_yoga__on_measure(YGNodeConstRef node, float width, YGMeasureMode width_mode, float height, YGMeasureMode height_mode) {
  int err;

  YGSize size = {.width = 0, .height = 0};

  bare_yoga_node_t *data = (bare_yoga_node_t *) YGNodeGetContext(node);

  if (data == NULL || data->measure == NULL) return size;

  js_env_t *env = data->env;

  bool pending;
  err = js_is_exception_pending(env, &pending);
  assert(err == 0);

  if (pending) return size;

  js_handle_scope_t *scope;
  err = js_open_handle_scope(env, &scope);
  assert(err == 0);

  js_value_t *fn;
  err = js_get_reference_value(env, data->measure, &fn);
  assert(err == 0);

  js_value_t *receiver;
  err = js_get_null(env, &receiver);
  assert(err == 0);

  js_value_t *argv[4];

  err = js_create_double(env, width, &argv[0]);
  assert(err == 0);

  err = js_create_int32(env, width_mode, &argv[1]);
  assert(err == 0);

  err = js_create_double(env, height, &argv[2]);
  assert(err == 0);

  err = js_create_int32(env, height_mode, &argv[3]);
  assert(err == 0);

  bare_yoga__measured[0] = 0;
  bare_yoga__measured[1] = 0;

  err = js_call_function(env, receiver, fn, 4, argv, NULL);

  if (err == 0) {
    size.width = (float) bare_yoga__measured[0];
    size.height = (float) bare_yoga__measured[1];
  }

  err = js_close_handle_scope(env, scope);
  assert(err == 0);

  return size;
}

static js_value_t *
bare_yoga_measured(js_env_t *env, js_callback_info_t *info) {
  int err;

  js_value_t *result;
  err = js_create_external_arraybuffer(env, bare_yoga__measured, sizeof(bare_yoga__measured), NULL, NULL, &result);
  assert(err == 0);

  return result;
}

static js_value_t *
bare_yoga_node_init(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 1;
  js_value_t *argv[1];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  assert(argc == 1);

  uint32_t config_tag;
  if (!bare_yoga__read_uint32(env, argv[0], "config", &config_tag)) return NULL;

  YGConfigRef config = bare_yoga__config(config_tag);

  YGNodeRef node = config == NULL ? YGNodeNew() : YGNodeNewWithConfig(config);

  bare_yoga_node_t *data = malloc(sizeof(bare_yoga_node_t));
  assert(data != NULL);

  data->env = env;
  data->measure = NULL;

  YGNodeSetContext(node, data);

  js_value_t *result;
  err = js_create_uint32(env, bare_yoga__registry_insert(&bare_yoga__nodes, node), &result);
  assert(err == 0);

  return result;
}

static js_value_t *
bare_yoga_node_destroy(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 1;
  js_value_t *argv[1];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  assert(argc == 1);

  uint32_t tag;
  if (!bare_yoga__read_uint32(env, argv[0], "node", &tag)) return NULL;

  YGNodeRef node = (YGNodeRef) bare_yoga__registry_remove(&bare_yoga__nodes, tag);

  if (node == NULL) return NULL;

  bare_yoga_node_t *data = (bare_yoga_node_t *) YGNodeGetContext(node);

  if (data != NULL) {
    if (data->measure != NULL) {
      err = js_delete_reference(env, data->measure);
      assert(err == 0);
    }

    free(data);
  }

  YGNodeFree(node);

  return NULL;
}

static js_value_t *
bare_yoga_node_measure(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 2;
  js_value_t *argv[2];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  assert(argc == 2);

  uint32_t tag;
  if (!bare_yoga__read_uint32(env, argv[0], "node", &tag)) return NULL;

  YGNodeRef node = bare_yoga__node(tag);

  if (node == NULL) return NULL;

  bare_yoga_node_t *data = (bare_yoga_node_t *) YGNodeGetContext(node);

  if (data->measure != NULL) {
    err = js_delete_reference(env, data->measure);
    assert(err == 0);

    data->measure = NULL;
  }

  js_value_type_t type;
  err = js_typeof(env, argv[1], &type);
  assert(err == 0);

  if (type == js_function) {
    if (!bare_yoga__check(env, YGNodeGetChildCount(node) == 0, "Only a leaf node can measure")) return NULL;

    err = js_create_reference(env, argv[1], 1, &data->measure);
    assert(err == 0);

    YGNodeSetMeasureFunc(node, bare_yoga__on_measure);
  } else {
    YGNodeSetMeasureFunc(node, NULL);
  }

  return NULL;
}

#define BARE_YOGA_TREE_2(name, body) \
  static js_value_t * \
  bare_yoga_node_##name(js_env_t *env, js_callback_info_t *info) { \
    int err; \
\
    size_t argc = 2; \
    js_value_t *argv[2]; \
\
    err = js_get_callback_info(env, info, &argc, argv, NULL, NULL); \
    assert(err == 0); \
\
    assert(argc == 2); \
\
    uint32_t parent_tag; \
    if (!bare_yoga__read_uint32(env, argv[0], "node", &parent_tag)) return NULL; \
\
    uint32_t child_tag; \
    if (!bare_yoga__read_uint32(env, argv[1], "child", &child_tag)) return NULL; \
\
    YGNodeRef node = bare_yoga__node(parent_tag); \
    YGNodeRef child = bare_yoga__node(child_tag); \
\
    if (node != NULL && child != NULL) body; \
\
    return NULL; \
  } \
\
  static void \
  bare_yoga_node_##name##_typed(js_value_t *receiver, uint32_t parent_tag, uint32_t child_tag, js_typed_callback_info_t *info) { \
    YGNodeRef node = bare_yoga__node(parent_tag); \
    YGNodeRef child = bare_yoga__node(child_tag); \
\
    if (node != NULL && child != NULL) body; \
  }

BARE_YOGA_TREE_2(remove_child, YGNodeRemoveChild(node, child))

static js_value_t *
bare_yoga_node_insert_child(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 3;
  js_value_t *argv[3];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  assert(argc == 3);

  uint32_t parent_tag;
  if (!bare_yoga__read_uint32(env, argv[0], "node", &parent_tag)) return NULL;

  uint32_t child_tag;
  if (!bare_yoga__read_uint32(env, argv[1], "child", &child_tag)) return NULL;

  uint32_t index;
  if (!bare_yoga__read_uint32(env, argv[2], "index", &index)) return NULL;

  YGNodeRef node = bare_yoga__node(parent_tag);
  YGNodeRef child = bare_yoga__node(child_tag);

  if (node == NULL || child == NULL) return NULL;

  if (!bare_yoga__check(env, YGNodeGetOwner(child) == NULL, "Child already has a parent")) return NULL;
  if (!bare_yoga__check(env, !YGNodeHasMeasureFunc(node), "A measured node cannot have children")) return NULL;
  if (!bare_yoga__check(env, index <= YGNodeGetChildCount(node), "Index is out of range")) return NULL;

  YGNodeInsertChild(node, child, index);

  return NULL;
}

static js_value_t *
bare_yoga_node_set_children(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 4;
  js_value_t *argv[4];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  assert(argc == 4);

  uint32_t tag;
  if (!bare_yoga__read_uint32(env, argv[0], "node", &tag)) return NULL;

  uint32_t offset;
  if (!bare_yoga__read_uint32(env, argv[2], "offset", &offset)) return NULL;

  uint32_t count;
  if (!bare_yoga__read_uint32(env, argv[3], "count", &count)) return NULL;

  uint32_t *tags;
  if (!bare_yoga__buffer(env, argv[1], offset, count, sizeof(uint32_t), (void **) &tags)) {
    err = js_throw_type_error(env, NULL, "Expected an array buffer with room for the child tags");
    assert(err == 0);

    return NULL;
  }

  YGNodeRef node = bare_yoga__node(tag);

  if (node == NULL) return NULL;

  YGNodeRef *children = count == 0 ? NULL : malloc(count * sizeof(YGNodeRef));

  for (uint32_t i = 0; i < count; i++) {
    YGNodeRef child = bare_yoga__node(tags[i]);

    if (child == NULL) {
      free(children);

      err = js_throw_type_errorf(env, NULL, "Unknown node %u", tags[i]);
      assert(err == 0);

      return NULL;
    }

    YGNodeRef owner = YGNodeGetOwner(child);

    if (owner != NULL && owner != node) {
      free(children);

      err = js_throw_error(env, NULL, "Child already has a parent");
      assert(err == 0);

      return NULL;
    }

    children[i] = child;
  }

  YGNodeSetChildren(node, children, count);

  free(children);

  return NULL;
}

#define BARE_YOGA_QUERY(name, type, create, body) \
  static js_value_t * \
  bare_yoga_node_##name(js_env_t *env, js_callback_info_t *info) { \
    int err; \
\
    size_t argc = 1; \
    js_value_t *argv[1]; \
\
    err = js_get_callback_info(env, info, &argc, argv, NULL, NULL); \
    assert(err == 0); \
\
    assert(argc == 1); \
\
    uint32_t tag; \
    if (!bare_yoga__read_uint32(env, argv[0], "node", &tag)) return NULL; \
\
    YGNodeRef node = bare_yoga__node(tag); \
\
    type value = node == NULL ? 0 : (body); \
\
    js_value_t *result; \
    err = create(env, value, &result); \
    assert(err == 0); \
\
    return result; \
  }

BARE_YOGA_QUERY(child_count, uint32_t, js_create_uint32, (uint32_t) YGNodeGetChildCount(node))
BARE_YOGA_QUERY(is_dirty, bool, js_get_boolean, YGNodeIsDirty(node))
BARE_YOGA_QUERY(has_new_layout, bool, js_get_boolean, YGNodeGetHasNewLayout(node))
BARE_YOGA_QUERY(had_overflow, bool, js_get_boolean, YGNodeLayoutGetHadOverflow(node))
BARE_YOGA_QUERY(layout_direction, int32_t, js_create_int32, YGNodeLayoutGetDirection(node))

#define BARE_YOGA_ACTION(name, body) \
  static js_value_t * \
  bare_yoga_node_##name(js_env_t *env, js_callback_info_t *info) { \
    int err; \
\
    size_t argc = 1; \
    js_value_t *argv[1]; \
\
    err = js_get_callback_info(env, info, &argc, argv, NULL, NULL); \
    assert(err == 0); \
\
    assert(argc == 1); \
\
    uint32_t tag; \
    if (!bare_yoga__read_uint32(env, argv[0], "node", &tag)) return NULL; \
\
    YGNodeRef node = bare_yoga__node(tag); \
\
    if (node != NULL) body; \
\
    return NULL; \
  } \
\
  static void \
  bare_yoga_node_##name##_typed(js_value_t *receiver, uint32_t tag, js_typed_callback_info_t *info) { \
    YGNodeRef node = bare_yoga__node(tag); \
\
    if (node != NULL) body; \
  }

BARE_YOGA_ACTION(clear_new_layout, YGNodeSetHasNewLayout(node, false))
BARE_YOGA_ACTION(remove_all_children, YGNodeRemoveAllChildren(node))

static js_value_t *
bare_yoga_node_mark_dirty(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 1;
  js_value_t *argv[1];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  assert(argc == 1);

  uint32_t tag;
  if (!bare_yoga__read_uint32(env, argv[0], "node", &tag)) return NULL;

  YGNodeRef node = bare_yoga__node(tag);

  if (node == NULL) return NULL;

  if (!bare_yoga__check(env, YGNodeHasMeasureFunc(node), "Only a measured node can be marked dirty")) return NULL;

  YGNodeMarkDirty(node);

  return NULL;
}

static js_value_t *
bare_yoga_node_reset(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 1;
  js_value_t *argv[1];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  assert(argc == 1);

  uint32_t tag;
  if (!bare_yoga__read_uint32(env, argv[0], "node", &tag)) return NULL;

  YGNodeRef node = bare_yoga__node(tag);

  if (node == NULL) return NULL;

  if (!bare_yoga__check(env, YGNodeGetChildCount(node) == 0, "Cannot reset a node with children")) return NULL;
  if (!bare_yoga__check(env, YGNodeGetOwner(node) == NULL, "Cannot reset a node with a parent")) return NULL;

  YGNodeReset(node);

  return NULL;
}

#define BARE_YOGA_STYLE_ENUM(name, Name, Type) \
  static js_value_t * \
  bare_yoga_node_##name(js_env_t *env, js_callback_info_t *info) { \
    int err; \
\
    size_t argc = 2; \
    js_value_t *argv[2]; \
\
    err = js_get_callback_info(env, info, &argc, argv, NULL, NULL); \
    assert(err == 0); \
\
    assert(argc == 2); \
\
    uint32_t tag; \
    if (!bare_yoga__read_uint32(env, argv[0], "node", &tag)) return NULL; \
\
    int32_t value; \
    if (!bare_yoga__read_int32(env, argv[1], #name, &value)) return NULL; \
\
    YGNodeRef node = bare_yoga__node(tag); \
\
    if (node != NULL) YGNodeStyleSet##Name(node, (Type) value); \
\
    return NULL; \
  } \
\
  static void \
  bare_yoga_node_##name##_typed(js_value_t *receiver, uint32_t tag, int32_t value, js_typed_callback_info_t *info) { \
    YGNodeRef node = bare_yoga__node(tag); \
\
    if (node != NULL) YGNodeStyleSet##Name(node, (Type) value); \
  }

BARE_YOGA_STYLE_ENUM(direction, Direction, YGDirection)
BARE_YOGA_STYLE_ENUM(flex_direction, FlexDirection, YGFlexDirection)
BARE_YOGA_STYLE_ENUM(justify_content, JustifyContent, YGJustify)
BARE_YOGA_STYLE_ENUM(align_content, AlignContent, YGAlign)
BARE_YOGA_STYLE_ENUM(align_items, AlignItems, YGAlign)
BARE_YOGA_STYLE_ENUM(align_self, AlignSelf, YGAlign)
BARE_YOGA_STYLE_ENUM(position_type, PositionType, YGPositionType)
BARE_YOGA_STYLE_ENUM(flex_wrap, FlexWrap, YGWrap)
BARE_YOGA_STYLE_ENUM(overflow, Overflow, YGOverflow)
BARE_YOGA_STYLE_ENUM(display, Display, YGDisplay)

#define BARE_YOGA_STYLE_FLOAT(name, Name) \
  static js_value_t * \
  bare_yoga_node_##name(js_env_t *env, js_callback_info_t *info) { \
    int err; \
\
    size_t argc = 2; \
    js_value_t *argv[2]; \
\
    err = js_get_callback_info(env, info, &argc, argv, NULL, NULL); \
    assert(err == 0); \
\
    assert(argc == 2); \
\
    uint32_t tag; \
    if (!bare_yoga__read_uint32(env, argv[0], "node", &tag)) return NULL; \
\
    double value; \
    if (!bare_yoga__read_double(env, argv[1], #name, &value)) return NULL; \
\
    YGNodeRef node = bare_yoga__node(tag); \
\
    if (node != NULL) YGNodeStyleSet##Name(node, (float) value); \
\
    return NULL; \
  } \
\
  static void \
  bare_yoga_node_##name##_typed(js_value_t *receiver, uint32_t tag, double value, js_typed_callback_info_t *info) { \
    YGNodeRef node = bare_yoga__node(tag); \
\
    if (node != NULL) YGNodeStyleSet##Name(node, (float) value); \
  }

BARE_YOGA_STYLE_FLOAT(flex, Flex)
BARE_YOGA_STYLE_FLOAT(flex_grow, FlexGrow)
BARE_YOGA_STYLE_FLOAT(flex_shrink, FlexShrink)
BARE_YOGA_STYLE_FLOAT(aspect_ratio, AspectRatio)

#define BARE_YOGA_APPLY_DIMENSION_AUTO(Name, node, value, unit) \
  switch (unit) { \
  case YGUnitPoint: \
    YGNodeStyleSet##Name(node, (float) value); \
    break; \
  case YGUnitPercent: \
    YGNodeStyleSet##Name##Percent(node, (float) value); \
    break; \
  case YGUnitAuto: \
    YGNodeStyleSet##Name##Auto(node); \
    break; \
  default: \
    YGNodeStyleSet##Name(node, YGUndefined); \
  }

#define BARE_YOGA_APPLY_DIMENSION(Name, node, value, unit) \
  switch (unit) { \
  case YGUnitPoint: \
    YGNodeStyleSet##Name(node, (float) value); \
    break; \
  case YGUnitPercent: \
    YGNodeStyleSet##Name##Percent(node, (float) value); \
    break; \
  default: \
    YGNodeStyleSet##Name(node, YGUndefined); \
  }

#define BARE_YOGA_STYLE_DIMENSION(name, Name, APPLY) \
  static js_value_t * \
  bare_yoga_node_##name(js_env_t *env, js_callback_info_t *info) { \
    int err; \
\
    size_t argc = 3; \
    js_value_t *argv[3]; \
\
    err = js_get_callback_info(env, info, &argc, argv, NULL, NULL); \
    assert(err == 0); \
\
    assert(argc == 3); \
\
    uint32_t tag; \
    if (!bare_yoga__read_uint32(env, argv[0], "node", &tag)) return NULL; \
\
    double value; \
    if (!bare_yoga__read_double(env, argv[1], #name, &value)) return NULL; \
\
    int32_t unit; \
    if (!bare_yoga__read_int32(env, argv[2], "unit", &unit)) return NULL; \
\
    YGNodeRef node = bare_yoga__node(tag); \
\
    if (node != NULL) APPLY(Name, node, value, unit) \
\
    return NULL; \
  } \
\
  static void \
  bare_yoga_node_##name##_typed(js_value_t *receiver, uint32_t tag, double value, int32_t unit, js_typed_callback_info_t *info) { \
    YGNodeRef node = bare_yoga__node(tag); \
\
    if (node != NULL) APPLY(Name, node, value, unit) \
  }

BARE_YOGA_STYLE_DIMENSION(width, Width, BARE_YOGA_APPLY_DIMENSION_AUTO)
BARE_YOGA_STYLE_DIMENSION(height, Height, BARE_YOGA_APPLY_DIMENSION_AUTO)
BARE_YOGA_STYLE_DIMENSION(flex_basis, FlexBasis, BARE_YOGA_APPLY_DIMENSION_AUTO)
BARE_YOGA_STYLE_DIMENSION(min_width, MinWidth, BARE_YOGA_APPLY_DIMENSION)
BARE_YOGA_STYLE_DIMENSION(min_height, MinHeight, BARE_YOGA_APPLY_DIMENSION)
BARE_YOGA_STYLE_DIMENSION(max_width, MaxWidth, BARE_YOGA_APPLY_DIMENSION)
BARE_YOGA_STYLE_DIMENSION(max_height, MaxHeight, BARE_YOGA_APPLY_DIMENSION)

#define BARE_YOGA_APPLY_EDGE_AUTO(Name, Type, node, edge, value, unit) \
  switch (unit) { \
  case YGUnitPoint: \
    YGNodeStyleSet##Name(node, (Type) edge, (float) value); \
    break; \
  case YGUnitPercent: \
    YGNodeStyleSet##Name##Percent(node, (Type) edge, (float) value); \
    break; \
  case YGUnitAuto: \
    YGNodeStyleSet##Name##Auto(node, (Type) edge); \
    break; \
  default: \
    YGNodeStyleSet##Name(node, (Type) edge, YGUndefined); \
  }

#define BARE_YOGA_APPLY_EDGE(Name, Type, node, edge, value, unit) \
  switch (unit) { \
  case YGUnitPoint: \
    YGNodeStyleSet##Name(node, (Type) edge, (float) value); \
    break; \
  case YGUnitPercent: \
    YGNodeStyleSet##Name##Percent(node, (Type) edge, (float) value); \
    break; \
  default: \
    YGNodeStyleSet##Name(node, (Type) edge, YGUndefined); \
  }

#define BARE_YOGA_STYLE_EDGE(name, Name, Type, APPLY) \
  static js_value_t * \
  bare_yoga_node_##name(js_env_t *env, js_callback_info_t *info) { \
    int err; \
\
    size_t argc = 4; \
    js_value_t *argv[4]; \
\
    err = js_get_callback_info(env, info, &argc, argv, NULL, NULL); \
    assert(err == 0); \
\
    assert(argc == 4); \
\
    uint32_t tag; \
    if (!bare_yoga__read_uint32(env, argv[0], "node", &tag)) return NULL; \
\
    int32_t edge; \
    if (!bare_yoga__read_int32(env, argv[1], "edge", &edge)) return NULL; \
\
    double value; \
    if (!bare_yoga__read_double(env, argv[2], #name, &value)) return NULL; \
\
    int32_t unit; \
    if (!bare_yoga__read_int32(env, argv[3], "unit", &unit)) return NULL; \
\
    YGNodeRef node = bare_yoga__node(tag); \
\
    if (node != NULL) APPLY(Name, Type, node, edge, value, unit) \
\
    return NULL; \
  } \
\
  static void \
  bare_yoga_node_##name##_typed(js_value_t *receiver, uint32_t tag, int32_t edge, double value, int32_t unit, js_typed_callback_info_t *info) { \
    YGNodeRef node = bare_yoga__node(tag); \
\
    if (node != NULL) APPLY(Name, Type, node, edge, value, unit) \
  }

BARE_YOGA_STYLE_EDGE(margin, Margin, YGEdge, BARE_YOGA_APPLY_EDGE_AUTO)
BARE_YOGA_STYLE_EDGE(padding, Padding, YGEdge, BARE_YOGA_APPLY_EDGE)
BARE_YOGA_STYLE_EDGE(position, Position, YGEdge, BARE_YOGA_APPLY_EDGE)
BARE_YOGA_STYLE_EDGE(gap, Gap, YGGutter, BARE_YOGA_APPLY_EDGE)

static js_value_t *
bare_yoga_node_border(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 3;
  js_value_t *argv[3];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  assert(argc == 3);

  uint32_t tag;
  if (!bare_yoga__read_uint32(env, argv[0], "node", &tag)) return NULL;

  int32_t edge;
  if (!bare_yoga__read_int32(env, argv[1], "edge", &edge)) return NULL;

  double value;
  if (!bare_yoga__read_double(env, argv[2], "border", &value)) return NULL;

  YGNodeRef node = bare_yoga__node(tag);

  if (node != NULL) YGNodeStyleSetBorder(node, (YGEdge) edge, (float) value);

  return NULL;
}

static void
bare_yoga_node_border_typed(js_value_t *receiver, uint32_t tag, int32_t edge, double value, js_typed_callback_info_t *info) {
  YGNodeRef node = bare_yoga__node(tag);

  if (node != NULL) YGNodeStyleSetBorder(node, (YGEdge) edge, (float) value);
}

static js_value_t *
bare_yoga_node_calculate_layout(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 4;
  js_value_t *argv[4];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  assert(argc == 4);

  uint32_t tag;
  if (!bare_yoga__read_uint32(env, argv[0], "node", &tag)) return NULL;

  double width;
  if (!bare_yoga__read_double(env, argv[1], "width", &width)) return NULL;

  double height;
  if (!bare_yoga__read_double(env, argv[2], "height", &height)) return NULL;

  int32_t direction;
  if (!bare_yoga__read_int32(env, argv[3], "direction", &direction)) return NULL;

  YGNodeRef node = bare_yoga__node(tag);

  if (node != NULL) YGNodeCalculateLayout(node, (float) width, (float) height, (YGDirection) direction);

  return NULL;
}

static void
bare_yoga_node_calculate_layout_typed(js_value_t *receiver, uint32_t tag, double width, double height, int32_t direction, js_typed_callback_info_t *info) {
  YGNodeRef node = bare_yoga__node(tag);

  if (node != NULL) YGNodeCalculateLayout(node, (float) width, (float) height, (YGDirection) direction);
}

static inline void
bare_yoga__write_layout(YGNodeRef node, double *out) {
  out[0] = YGNodeLayoutGetLeft(node);
  out[1] = YGNodeLayoutGetTop(node);
  out[2] = YGNodeLayoutGetWidth(node);
  out[3] = YGNodeLayoutGetHeight(node);
}

static js_value_t *
bare_yoga_node_layout(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 3;
  js_value_t *argv[3];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  assert(argc == 3);

  uint32_t tag;
  if (!bare_yoga__read_uint32(env, argv[0], "node", &tag)) return NULL;

  uint32_t offset;
  if (!bare_yoga__read_uint32(env, argv[2], "offset", &offset)) return NULL;

  double *out;
  if (!bare_yoga__buffer(env, argv[1], offset, 4, sizeof(double), (void **) &out)) {
    err = js_throw_type_error(env, NULL, "Expected an array buffer with room for 4 doubles");
    assert(err == 0);

    return NULL;
  }

  YGNodeRef node = bare_yoga__node(tag);

  if (node != NULL) bare_yoga__write_layout(node, out);

  return NULL;
}

static void
bare_yoga_node_layout_typed(js_value_t *receiver, uint32_t tag, js_value_t *buffer, uint32_t offset, js_typed_callback_info_t *info) {
  int err;

  js_env_t *env;
  err = js_get_typed_callback_info(info, &env, NULL);
  assert(err == 0);

  double *out;
  if (!bare_yoga__buffer(env, buffer, offset, 4, sizeof(double), (void **) &out)) return;

  YGNodeRef node = bare_yoga__node(tag);

  if (node != NULL) bare_yoga__write_layout(node, out);
}

static js_value_t *
bare_yoga_node_layout_many(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 5;
  js_value_t *argv[5];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  assert(argc == 5);

  uint32_t tags_offset;
  if (!bare_yoga__read_uint32(env, argv[1], "tagsOffset", &tags_offset)) return NULL;

  uint32_t count;
  if (!bare_yoga__read_uint32(env, argv[2], "count", &count)) return NULL;

  uint32_t out_offset;
  if (!bare_yoga__read_uint32(env, argv[4], "offset", &out_offset)) return NULL;

  uint32_t *tags;
  if (!bare_yoga__buffer(env, argv[0], tags_offset, count, sizeof(uint32_t), (void **) &tags)) {
    err = js_throw_type_error(env, NULL, "Expected an array buffer with room for the node tags");
    assert(err == 0);

    return NULL;
  }

  double *out;
  if (!bare_yoga__buffer(env, argv[3], out_offset, count * 5, sizeof(double), (void **) &out)) {
    err = js_throw_type_error(env, NULL, "Expected an array buffer with room for 5 doubles per node");
    assert(err == 0);

    return NULL;
  }

  for (uint32_t i = 0; i < count; i++) {
    YGNodeRef node = bare_yoga__node(tags[i]);

    if (node == NULL) {
      err = js_throw_type_errorf(env, NULL, "Unknown node %u", tags[i]);
      assert(err == 0);

      return NULL;
    }

    out[i * 5] = YGNodeGetHasNewLayout(node) ? 1 : 0;

    bare_yoga__write_layout(node, out + i * 5 + 1);
  }

  return NULL;
}

static js_value_t *
bare_yoga_node_registry_size(js_env_t *env, js_callback_info_t *info) {
  int err;

  js_value_t *result;
  err = js_create_uint32(env, bare_yoga__nodes.size, &result);
  assert(err == 0);

  return result;
}
