#pragma once

#include <assert.h>
#include <js.h>
#include <stdint.h>
#include <stdlib.h>

#include <yoga/Yoga.h>

#include "registry.h"

// The state of one instance of the addon. A process can load the addon more
// than once, such as for each graph a development build replaces, and tags
// are plain integers, so state shared between instances would let one hand
// out the tags of another and reset them when it loads.
typedef struct {
  bare_yoga_registry_t nodes;
  bare_yoga_registry_t configs;

  // Where a measure function writes its result, read back once it returns.
  double measured[2];

  uint32_t refs;
} bare_yoga_instance_t;

// The node data is defined with the node bindings, but freeing what an
// instance still holds needs to reach its measure function.
typedef struct {
  js_env_t *env;
  js_ref_t *measure;
  bare_yoga_instance_t *instance;
} bare_yoga_node_t;

static inline bare_yoga_instance_t *
bare_yoga__typed_instance(js_typed_callback_info_t *info) {
  js_env_t *env;
  void *data;
  int err = js_get_typed_callback_info(info, &env, &data);
  assert(err == 0);

  return (bare_yoga_instance_t *) data;
}

static inline void
bare_yoga__instance_retain(bare_yoga_instance_t *instance) {
  instance->refs++;
}

// Called from a finalizer rather than a teardown callback, so the environment
// is still there for the measure functions of nodes that were never destroyed.
static void
bare_yoga__instance_release(bare_yoga_instance_t *instance) {
  int err;

  assert(instance->refs > 0);

  if (--instance->refs > 0) return;

  for (uint32_t tag = 1; tag < instance->nodes.len; tag++) {
    YGNodeRef node = (YGNodeRef) instance->nodes.slots[tag].value;

    if (node == NULL) continue;

    bare_yoga_node_t *data = (bare_yoga_node_t *) YGNodeGetContext(node);

    if (data != NULL) {
      if (data->measure != NULL) {
        err = js_delete_reference(data->env, data->measure);
        assert(err == 0);
      }

      free(data);
    }

    YGNodeFree(node);
  }

  for (uint32_t tag = 1; tag < instance->configs.len; tag++) {
    YGConfigRef config = (YGConfigRef) instance->configs.slots[tag].value;

    if (config != NULL) YGConfigFree(config);
  }

  bare_yoga__registry_destroy(&instance->nodes);
  bare_yoga__registry_destroy(&instance->configs);

  free(instance);
}

static void
bare_yoga__on_instance_release(js_env_t *env, void *data, void *finalize_hint) {
  bare_yoga__instance_release((bare_yoga_instance_t *) finalize_hint);
}

// The instance lives as long as `exports` and anything else that retains it.
static bare_yoga_instance_t *
bare_yoga__instance_create(js_env_t *env, js_value_t *exports) {
  int err;

  bare_yoga_instance_t *instance = malloc(sizeof(bare_yoga_instance_t));
  assert(instance != NULL);

  bare_yoga__registry_init(&instance->nodes);
  bare_yoga__registry_init(&instance->configs);

  instance->measured[0] = 0;
  instance->measured[1] = 0;
  instance->refs = 1;

  err = js_add_finalizer(env, exports, instance, bare_yoga__on_instance_release, instance, NULL);
  assert(err == 0);

  return instance;
}
