#ifndef TABLE_H
#define TABLE_H

#include "common.h"
#include "value.h"

struct Entry {
  struct ObjectString *key;
  struct Value value;
};

struct Table {
  size_t count;
  size_t capacity;
  struct Entry *entries;
};

void table_init(struct Table *table);
void table_free(struct Table *table);
uint8_t table_set(struct Table *table, struct ObjectString *key, struct Value value);
uint8_t table_get(struct Table *table, struct ObjectString *key, struct Value *out);
uint8_t table_remove(struct Table *table, struct ObjectString *key);
void table_set_all_from(struct Table *dest, struct Table *src);

#endif // TABLE_H