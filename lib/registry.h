#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
  void *value;
  uint32_t next;
} bare_yoga__slot_t;

typedef struct {
  bare_yoga__slot_t *slots;
  uint32_t len;
  uint32_t capacity;
  uint32_t free;
  uint32_t size;
} bare_yoga_registry_t;

static inline void
bare_yoga__registry_init(bare_yoga_registry_t *registry) {
  registry->slots = NULL;
  registry->len = 1;
  registry->capacity = 0;
  registry->free = 0;
  registry->size = 0;
}

static inline uint32_t
bare_yoga__registry_insert(bare_yoga_registry_t *registry, void *value) {
  uint32_t tag;

  if (registry->free != 0) {
    tag = registry->free;
    registry->free = registry->slots[tag].next;
  } else {
    if (registry->len >= registry->capacity) {
      uint32_t capacity = registry->capacity < 16 ? 16 : registry->capacity * 2;

      registry->slots = realloc(registry->slots, capacity * sizeof(bare_yoga__slot_t));
      assert(registry->slots != NULL);

      registry->capacity = capacity;
    }

    tag = registry->len++;
  }

  registry->slots[tag].value = value;
  registry->size++;

  return tag;
}

static inline void *
bare_yoga__registry_get(bare_yoga_registry_t *registry, uint32_t tag) {
  if (tag == 0 || tag >= registry->len) return NULL;

  return registry->slots[tag].value;
}

static inline void *
bare_yoga__registry_remove(bare_yoga_registry_t *registry, uint32_t tag) {
  void *value = bare_yoga__registry_get(registry, tag);

  if (value == NULL) return NULL;

  registry->slots[tag].value = NULL;
  registry->slots[tag].next = registry->free;
  registry->free = tag;
  registry->size--;

  return value;
}
