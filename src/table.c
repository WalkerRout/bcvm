
#include <stdio.h>

#include "table.h"
#include "memory.h"
#include "value.h"
#include "object.h"

#define TABLE_MAX_LOAD 0.75

// file local prototypes
static void table_adjust_capacity(struct Table *table, size_t capacity);
static struct Entry *find_entry(struct Entry *entries, size_t capacity, struct ObjectString *key);

void table_init(struct Table *table) {
  table->count    = 0;
  table->capacity = 0;
  table->entries  = NULL;
}

void table_free(struct Table *table) {
  MEMORY_FREE_ARRAY(struct Entry, table->entries, table->capacity);
  table_init(table);
}

uint8_t table_set(struct Table *table, struct ObjectString *key, struct Value value) {
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    size_t capacity = MEMORY_GROW_CAPACITY(table->capacity);
    adjust_capacity(table, capacity);
  }

  struct Entry *entry = find_entry(table->entries, table->capacity, key);
  uint8_t is_new_key = entry->key == NULL;
  if (is_new_key) table->count++;

  entry->key = key;
  entry->value = value;
  return is_new_key;
}

// file local functions

static void table_adjust_capacity(struct Table *table, size_t capacity) {
  struct Entry *entries = MEMORY_ALLOCATE(struct Entry, capacity);
  for (size_t i = 0; i < capacity; i++) {
    entries[i].key = NULL;
    entries[i].value = NIL_VAL;
  }

  for (size_t i = 0; i < table->capacity; i++) {
    struct Entry *entry = &table->entries[i];
    if (entry->key == NULL) continue;

    struct Entry *dest = find_entry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
  }

  MEMORY_FREE_ARRAY(struct Entry, table->entries, table->capacity);

  table->entries = entries;
  table->capacity = capacity;
}

static struct Entry *find_entry(struct Entry *entries, size_t capacity, struct ObjectString *key) {
  uint32_t index = key->hash % capacity;
  for (;;) {
    struct Entry *entry = &entries[index];
    if (entry->key == key || entry->key == NULL) {
      return entry;
    }

    index = (index + 1) % capacity;
  }
}