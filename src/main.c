#include <stdio.h>
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

  printf("%p\n", table);
  printf("%ld\n", table->capacity);

  int idx = kv_put(table, "hehe", "haha");
  printf("1.idx: %d\n", idx);
  idx = kv_put(table, "hehe", "hoho");
  printf("2.idx: %d\n", idx);
  idx = kv_put(table, "lala", "hoho");
  printf("3.idx: %d\n", idx);

  for (int i = 0; i < table->capacity; i++) {
    if (table->entries[i].key) {
      printf("[%d]%s: %s\n", i, table->entries[i].key, table->entries[i].value);
    }
  }

  return 0;
}
