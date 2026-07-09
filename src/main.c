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
  kv_t *table = kv_init(3);

  printf("%p\n", table);

  printf("%ld\n", table->capacity);

  return 0;
}
