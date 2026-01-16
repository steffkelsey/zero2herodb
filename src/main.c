#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
  printf("Usage: %s -n -f <database file>\n", argv[0]);
  printf("\t -n - create new database file\n");
  printf("\t -f - [required] path to database file\n");
  printf("\t -l - list the employees\n");
  printf("\t -a - add via CSV list of (name,address,hours)\n");
  return;
}

int main(int argc, char *argv[]) { 
  char *addstring = NULL;
  char *removestring = NULL;
  char *updatestring = NULL;
  char *filepath = NULL;
  bool newfile = false;
  bool list = false;
  int c;

  int dbfd = -1;
  struct dbheader_t *dbhdr = NULL;
  struct employee_t *employees = NULL;

  while ((c = getopt(argc, argv, "nlf:a:r:u:")) != -1) {
    switch (c) {
      case 'a':
        addstring = optarg;
        break;
      case 'f':
        filepath = optarg;
        break;
      case 'n':
        newfile = true;
        break;
      case 'l':
        list = true;
        break;
      case 'r':
        removestring = optarg;
        break;
      case 'u':
        updatestring = optarg;
        break;
      case '?':
        printf("Unknown option -%c\n", c);
        break;
      default:
        return STATUS_ERROR;
    } 
  }

  if (filepath == NULL) {
    printf("Filepath is a required argument\n");
    print_usage(argv);

    return 0;
  }

  if (newfile) {
    dbfd = create_db_file(filepath);
    if (dbfd == STATUS_ERROR) {
      printf("Unable to create database file\n");
      return -1;
    }

    if (create_db_header(&dbhdr) == STATUS_ERROR) {
      printf("Failed to create database header\n");
      return -1;
    }
  } else {
    dbfd = open_db_file(filepath);
    if (dbfd == STATUS_ERROR) {
      printf("Unable to open database file\n");
      return -1;
    }
    if (validate_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
      printf("Failed to validate database header\n");
      return -1;
    }
  }

  if (read_employees(dbfd, dbhdr, &employees) == STATUS_ERROR) {
    printf("Failed to read employees\n");
    return -1;
  }

  if (addstring) {
    if (add_employee(dbhdr, &employees, addstring) == STATUS_ERROR) {
      printf("Failed to add employee\n");
      return -1;
    }
  }

  if (updatestring) {
    if (update_employee(dbhdr, employees, updatestring) == STATUS_ERROR) {
      printf("Failed to update employee\n");
      return -1;
    }
  }

  if (removestring) {
    if (remove_employee(dbhdr, &employees, removestring) == STATUS_ERROR) {
      printf("Failed to remove employee\n");
      return -1;
    }
  }

  if (list && dbhdr) {
    list_employees(dbhdr, employees);
  }

  output_file(dbfd, dbhdr, employees);

  free(dbhdr);
  dbhdr = NULL;

  free(employees);
  employees = NULL;
	
  return 0;
}
