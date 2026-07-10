#include <kv.h>
#include <string.h>
#include <stdio.h>

// fn hash
// params: 
//   - val: a pointer to the value to hash
//   - capacity: the capacity of the db
// returns: 
size_t hash(char *val, int capacity) {
  size_t hash = 0x13371337deadbeef;

  while(*val) {
    
    hash ^= *val;
    hash = hash << 8;
    hash += *val;

    val++;
  }

  return hash % capacity;
}

// fn kv_put
// params:
//   - db: a pointer to the db
//   - key: a pointer to the key
//   - value: a pointer to the value
// returns: the index of the key, otherwise on 
// error, returns -1. on not found, return -2
int kv_put(kv_t *db, char *key, char *value) {
  if (!db || !key || !value) return -1;

  size_t idx = hash(key, db->capacity);

  for (int i = 0; i < db->capacity - 1; i++) {
    size_t real_idx = (idx + i) % db->capacity;

    kv_entry_t *entry = &db->entries[real_idx];

    // the key is already set, updating
    if (entry->key && 
        entry->key != TOMBSTONE &&
        !strcmp(entry->key, key)) {
      char *newval = strdup(value);
      if (!newval) return -1;
      entry->value = newval;
      return 0;
    }

    // land in a slot that is "empty"
    // null or tombstone
    if (!entry->key || entry->key == TOMBSTONE) {
      char *newval = strdup(value);
      char *newkey = strdup(key);
      if (!newval || !newkey) {
        free(newkey);
        free(newval);
        return -1;
      }
      entry->key = newkey;
      entry->value = newval;
      db->count++;
      return 0;
    }
  }

  // the db is occupied;
  return -2;
}

kv_t *kv_init(size_t capacity) {
  if (capacity == 0) return NULL;

  kv_t *table = malloc(sizeof(kv_t));
  if (table == NULL) {
    return NULL;
  }

  table->capacity = capacity;
  table->count = 0;

  table->entries = calloc(sizeof(kv_entry_t), capacity);  
  if (table->entries == NULL) {
    return NULL;
  }

  return table;
}
