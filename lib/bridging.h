#pragma once

#include <assert.h>
#include <js.h>
#include <stdbool.h>
#include <stdint.h>

static bool
bare_yoga__read_number(js_env_t *env, js_value_t *value, const char *name) {
  int err;

  js_value_type_t type;
  err = js_typeof(env, value, &type);
  assert(err == 0);

  if (type == js_number) return true;

  err = js_throw_type_errorf(env, NULL, "Expected a number for '%s'", name);
  assert(err == 0);

  return false;
}

static bool
bare_yoga__read_double(js_env_t *env, js_value_t *value, const char *name, double *result) {
  if (!bare_yoga__read_number(env, value, name)) return false;

  int err = js_get_value_double(env, value, result);
  assert(err == 0);

  return true;
}

static bool
bare_yoga__read_int32(js_env_t *env, js_value_t *value, const char *name, int32_t *result) {
  if (!bare_yoga__read_number(env, value, name)) return false;

  int err = js_get_value_int32(env, value, result);
  assert(err == 0);

  return true;
}

static bool
bare_yoga__read_uint32(js_env_t *env, js_value_t *value, const char *name, uint32_t *result) {
  if (!bare_yoga__read_number(env, value, name)) return false;

  int err = js_get_value_uint32(env, value, result);
  assert(err == 0);

  return true;
}

static bool
bare_yoga__buffer(js_env_t *env, js_value_t *value, uint32_t offset, uint32_t count, size_t width, void **result) {
  int err;

  bool is_arraybuffer;
  err = js_is_arraybuffer(env, value, &is_arraybuffer);
  assert(err == 0);

  if (!is_arraybuffer) return false;

  void *data;
  size_t len;
  err = js_get_arraybuffer_info(env, value, &data, &len);
  assert(err == 0);

  if ((uint64_t) offset + (uint64_t) count * width > len) return false;

  *result = (uint8_t *) data + offset;

  return true;
}
