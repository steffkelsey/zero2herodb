#include <kv.h>
#include <string.h>
#include <stdio.h>

#define TOMBSTONE ((char *)0x1)

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

// fn kv_get
// params:
//   - db: a pointer to the db
//   - key: a pointer to the key
// returns: the pointer to the value
// NULL if not found
char *kv_get(kv_t *db, char *key) {
  if (!db || !key) return NULL;

  size_t idx = hash(key, db->capacity);

  for (int i = 0; i < db->capacity - 1; i++) {
    size_t real_idx = (idx + i) % db->capacity;

    kv_entry_t *entry = &db->entries[real_idx];

    // check if there is no key
    if (entry->key == NULL) {
      return NULL;
    }

    // find and entry and the keys match
    if (entry->key && 
        entry->key != TOMBSTONE &&
        !strcmp(entry->key, key)) {
      return entry->value;
    }
  }

  return NULL;
}

// fn kv_delete
// params:
//   - db: a pointer to the db
//   - key: a pointer to the key to delete
// returns: 0 if deleted sucessfully, -1 if error and 
// -1 if not found
int kv_delete(kv_t *db, char *key) {
  if (!db || !key) return -1;

  size_t idx = hash(key, db->capacity);

  for (int i = 0; i < db->capacity - 1; i++) {
    size_t real_idx = (idx + i) % db->capacity;

    kv_entry_t *entry = &db->entries[real_idx];

    // check if there is no key
    if (entry->key == NULL) {
      return -1;
    }

    // find and entry and the keys match
    if (entry->key && 
        entry->key != TOMBSTONE &&
        !strcmp(entry->key, key)) {
      free(entry->key);
      free(entry->value);
      db->count--;
      entry->key = TOMBSTONE;
      entry->value = NULL;
      return 0;
    }
  }

  // return -1 if not found
  return -1;
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
