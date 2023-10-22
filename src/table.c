
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
    size_t capacity = MEMORY_GROW_CAPACITY(table->capacity, 8);
    table_adjust_capacity(table, capacity);
  }

  struct Entry *entry = find_entry(table->entries, table->capacity, key);
  uint8_t is_new_key = entry->key == NULL;
  if (is_new_key && VALUE_IS_NIL(entry->value)) table->count += 1;

  entry->key = key;
  entry->value = value;
  return is_new_key;
}

uint8_t table_get(struct Table *table, struct ObjectString *key, struct Value *out) {
  if (table->count == 0) return FALSE;

  struct Entry *entry = find_entry(table->entries, table->capacity, key);
  if (entry->key == NULL) return FALSE;

  *out = entry->value;
  return TRUE;
}

uint8_t table_remove(struct Table *table, struct ObjectString *key) {
  if (table->count == 0) return FALSE;

  struct Entry *entry = find_entry(table->entries, table->capacity, key);
  if (entry->key == NULL) return FALSE;

  // tombstone entry for list continuity
  entry->key = NULL;
  entry->value = VALUE_BOOL(TRUE);
  return TRUE;
}

void table_set_all_from(struct Table *dest, struct Table *src) {
  for (size_t i = 0; i < src->capacity; ++i) {
    struct Entry *entry = &src->entries[i];
    if (entry->key != NULL) {
      table_set(dest, entry->key, entry->value);
    }
  }
}

// file local functions

static void table_adjust_capacity(struct Table *table, size_t capacity) {
  struct Entry *entries = MEMORY_ALLOCATE(struct Entry, capacity);
  for (size_t i = 0; i < capacity; ++i) {
    entries[i].key = NULL;
    entries[i].value = VALUE_NIL();
  }

  table->count = 0; // tombstones not included
  for (size_t i = 0; i < table->capacity; ++i) {
    struct Entry *entry = &table->entries[i];
    if (entry->key == NULL) continue;

    struct Entry *dest = find_entry(entries, capacity, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;

    // increment for non-tombstone entries
    table->count += 1;
  }

  MEMORY_FREE_ARRAY(struct Entry, table->entries, table->capacity);

  table->entries = entries;
  table->capacity = capacity;
}

static struct Entry *find_entry(struct Entry *entries, size_t capacity, struct ObjectString *key) {
  struct Entry *tombstone = NULL;
  uint32_t index = key->hash % capacity;
  for (;;) {
    struct Entry *entry = &entries[index];
    if (entry->key == NULL) {
      if (VALUE_IS_NIL(entry->value)) {
        // empty slot found
        return tombstone != NULL 
          ? tombstone
          : entry;
      } else {
        // tombstone found
        if (tombstone == NULL) tombstone = entry;
      }
    } else if (entry->key == key) {
      // key found
      return entry;
    }

    index = (index + 1) % capacity;
  }
}