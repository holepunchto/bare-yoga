#pragma once

#include <assert.h>
#include <js.h>
#include <stdbool.h>
#include <stdint.h>

#include <yoga/Yoga.h>

#include "bridging.h"
#include "registry.h"

static bare_yoga_registry_t bare_yoga__configs;

static inline YGConfigRef
bare_yoga__config(uint32_t tag) {
  return (YGConfigRef) bare_yoga__registry_get(&bare_yoga__configs, tag);
}

static js_value_t *
bare_yoga_config_init(js_env_t *env, js_callback_info_t *info) {
  int err;

  js_value_t *result;
  err = js_create_uint32(env, bare_yoga__registry_insert(&bare_yoga__configs, YGConfigNew()), &result);
  assert(err == 0);

  return result;
}

static js_value_t *
bare_yoga_config_destroy(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 1;
  js_value_t *argv[1];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  assert(argc == 1);

  uint32_t tag;
  if (!bare_yoga__read_uint32(env, argv[0], "config", &tag)) return NULL;

  YGConfigRef config = (YGConfigRef) bare_yoga__registry_remove(&bare_yoga__configs, tag);

  if (config != NULL) YGConfigFree(config);

  return NULL;
}

#define BARE_YOGA_CONFIG(name, Name, type, read, cast) \
  static js_value_t * \
  bare_yoga_config_##name(js_env_t *env, js_callback_info_t *info) { \
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
    if (!bare_yoga__read_uint32(env, argv[0], "config", &tag)) return NULL; \
\
    type value; \
    if (!read(env, argv[1], #name, &value)) return NULL; \
\
    YGConfigRef config = bare_yoga__config(tag); \
\
    if (config != NULL) YGConfigSet##Name(config, cast value); \
\
    return NULL; \
  }

BARE_YOGA_CONFIG(point_scale_factor, PointScaleFactor, double, bare_yoga__read_double, (float) )
BARE_YOGA_CONFIG(errata, Errata, int32_t, bare_yoga__read_int32, (YGErrata))

static js_value_t *
bare_yoga_config_use_web_defaults(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 2;
  js_value_t *argv[2];

  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);

  assert(argc == 2);

  uint32_t tag;
  if (!bare_yoga__read_uint32(env, argv[0], "config", &tag)) return NULL;

  bool value;
  err = js_get_value_bool(env, argv[1], &value);
  assert(err == 0);

  YGConfigRef config = bare_yoga__config(tag);

  if (config != NULL) YGConfigSetUseWebDefaults(config, value);

  return NULL;
}
