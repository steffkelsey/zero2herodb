#include <stdio.h>
#include <assert.h>
#include <kv.h>

void print_usage(char *argv[]) {
  printf("Usage: %s -n -f <database file>\n", argv[0]);
  printf("\t -n - create new database file\n");
  printf("\t -f - [required] path to database file\n");
  printf("\t -l - list the employees\n");
  printf("\t -a - add via CSV list of (name,address,hours)\n");
  return;
}

int main() { 
  kv_t *table = kv_init(1024);

  //printf("%p\n", table);
  //printf("%ld\n", table->capacity);

  kv_put(table, "hehe", "haha");
  //kv_put(table, "hehe", "hoho");
  kv_put(table, "lala", "hoho");

  for (int i = 0; i < table->capacity; i++) {
    if (table->entries[i].key) {
      printf("[%d]%s: %s\n", i, table->entries[i].key, table->entries[i].value);
    }
  }

  char *val1 = kv_get(table, "hehe");
  printf("kv_get('hehe'): %s\n", val1);

  char *val2 = kv_get(table, "lala");
  printf("kv_get('lala'): %s\n", val2);

  char *val3 = kv_get(table, "no_exist");
  printf("kv_get('no_exist'): %s\n", val3);
  
  kv_t *db = kv_init(16);

  kv_put(db, "name", "alice");
  kv_put(db, "city", "berlin");

  assert(kv_delete(db, "name") == 0);
  assert(kv_get(db, "name") == NULL);
  assert(db->count == 1);

  assert(kv_delete(db, "missing") == -1);

  //kv_free(db);

  return 0;
}
