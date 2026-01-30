#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "parse.h"

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
  if (NULL == dbhdr) return;
  int i = 0;
  for(; i < dbhdr->count; i++) {
    printf("Employee %d\n", i);
    printf("\tName: %s\n", employees[i].name);
    printf("\tAddress: %s\n", employees[i].address);
    printf("\tHours: %d\n", employees[i].hours);
  }
}

int add_employee(struct dbheader_t *dbhdr, struct employee_t **employees, char *addstring) {
  if (NULL == dbhdr) return STATUS_ERROR;
  if (NULL == employees) return STATUS_ERROR;
  if (NULL == *employees) return STATUS_ERROR;
  if (NULL == addstring) return STATUS_ERROR;

  char *name = strtok(addstring, ",");
  if (NULL == name) return STATUS_ERROR;

  char *addr = strtok(NULL, ",");
  if (NULL == addr) return STATUS_ERROR;

  char *hours = strtok(NULL, ",");
  if (NULL == hours) return STATUS_ERROR;

  struct employee_t *e = *employees;
  e = realloc(e, sizeof(struct employee_t)*(dbhdr->count+1));
  if (e == NULL) {
    return STATUS_ERROR;
  }

  dbhdr->count++;

  strncpy(e[dbhdr->count-1].name, name, sizeof(e[dbhdr->count-1].name));
  strncpy(e[dbhdr->count-1].address, addr, sizeof(e[dbhdr->count-1].address));
  e[dbhdr->count-1].hours = atoi(hours);

  *employees = e;

  return STATUS_SUCCESS;
}

int remove_employee(struct dbheader_t *dbhdr, struct employee_t **employees, char *removestring) {
  if (NULL == dbhdr) return STATUS_ERROR;
  if (NULL == employees) return STATUS_ERROR;
  if (NULL == *employees) return STATUS_ERROR;
  if (NULL == removestring) return STATUS_ERROR;

  if (NULL == removestring) return STATUS_ERROR;

  // Temp for iterating over employees
  struct employee_t *e = *employees;

  int newcount = dbhdr->count;
  int i = 0;
  for (; i < dbhdr->count; i++) {
    if (strcmp(e[i].name, removestring) == 0) {
      printf("%s found at %d!\n", removestring, i);
      newcount--;
    }
  }

  // If none to delete, then move on
  if (newcount == dbhdr->count) {
    printf("Found 0 records to delete\n");
    e = NULL;
    return STATUS_SUCCESS;
  }

  printf("new count is %d\n", newcount);

  if (newcount == 0) {
    // TODO What if we deleted all records?
    printf("Removing all records!\n");
  }

  // Allocate memory for a new array
  struct employee_t *newe = calloc(newcount, sizeof(struct employee_t));
  if (newe == NULL) {
    printf("Malloc failed to create employees\n");
    return STATUS_ERROR;
  }

  int index = 0;
  for (i = 0; i < dbhdr->count; i++) {
    printf("checking i=%d...\n", i);
    if (strcmp(e[i].name, removestring) != 0) {
      printf("copying %s...\n", e[i].name);
      newe[index] = e[i];
      index++;
    }
  }

  free(e);
  e = NULL;

  dbhdr->count = newcount;
  *employees = newe;

  printf("dbhdr->count = %d\n", dbhdr->count);
  return STATUS_SUCCESS;
}

int update_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *updatestring) {
  if (NULL == dbhdr) return STATUS_ERROR;
  if (NULL == employees) return STATUS_ERROR;
  if (NULL == updatestring) return STATUS_ERROR;

  char *name = strtok(updatestring, ",");
  if (NULL == name) return STATUS_ERROR;

  char *hours = strtok(NULL, ",");
  if (NULL == hours) return STATUS_ERROR;

  bool updated = false;
  int i = 0;
  for (; i < dbhdr->count; i++) {
    if (strcmp(employees[i].name, name) == 0) {
      employees[i].hours = atoi(hours);
      updated = true;
      break;
    }
  }

  if (!updated) {
    printf("No employee found with name '%s'\n", name);
    return STATUS_ERROR;    
  }

  return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
  if (fd < 0) {
    printf("Got a bnad FD from the user\n");
    return STATUS_ERROR;
  }

  int count = dbhdr->count;

  struct employee_t *employees = calloc(count, sizeof(struct employee_t));
  if (employees == NULL) {
    printf("Malloc failed to create employees\n");
    return STATUS_ERROR;
  }

  read(fd, employees, count*sizeof(struct employee_t));

  int i = 0;

  for (; i < count; i++) {
    employees[i].hours = ntohl(employees[i].hours);
  }

  *employeesOut = employees;

  return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
  if (fd < 0) {
    printf("Got a bad FD from the user\n");
    return STATUS_ERROR;
  }

  int realcount = dbhdr->count;

  printf("realcount: %d\n", realcount);

  dbhdr->version = htons(dbhdr->version);
  dbhdr->count = htons(dbhdr->count);
  dbhdr->magic = htonl(dbhdr->magic);
  dbhdr->filesize = htonl(sizeof(struct dbheader_t) + realcount*sizeof(struct employee_t));

  // Truncate the file to zero each time to always overwrite from scratch
  if (ftruncate(fd, 0) == -1) {
    printf("Error truncating file\n");
    return STATUS_ERROR;
  }

  lseek(fd, 0, SEEK_SET);
  write(fd, dbhdr, sizeof(struct dbheader_t));

  int i = 0;

  for (; i < realcount; i++) {
    employees[i].hours = htonl(employees[i].hours);
    write(fd, &employees[i], sizeof(struct employee_t));
  }

  return STATUS_SUCCESS;
}	

int validate_db_header(int fd, struct dbheader_t **headerOut) {
  if (fd < 0) {
    printf("Got a bnad FD from the user\n");
    return STATUS_ERROR;
  }
  
  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
  if (header == NULL) {
    printf("Malloc failed to create db header\n");
    return STATUS_ERROR;
  }

  if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
    perror("read");
    free(header);
    return STATUS_ERROR;
  }

  header->version = ntohs(header->version);
  header->count = ntohs(header->count);
  header->magic = ntohl(header->magic);
  header->filesize = ntohl(header->filesize);

  if (header->magic != HEADER_MAGIC) {
    printf("Improper header magic\n");
    free(header);
    return STATUS_ERROR;
  }

  if (header->version != 1) {
    printf("Improper header version\n");
    free(header);
    return STATUS_ERROR;
  }

  struct stat dbstat = {0};
  fstat(fd, &dbstat);
  printf("header->filesize: %d, st_size: %d\n", header->filesize, dbstat.st_size);
  if (header->filesize != dbstat.st_size) {
    printf("Corrupted database\n");
    free(header);
    return STATUS_ERROR;
  }

  *headerOut = header;
}

int create_db_header(struct dbheader_t **headerOut) {
  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
  if (header == NULL) {
    printf("Malloc failed to create db header\n");
    return STATUS_ERROR;
  }
  header->version = 0x1;
  header->count = 0;
  header->magic = HEADER_MAGIC;
  header->filesize = sizeof(struct dbheader_t);

  *headerOut = header;

  return STATUS_SUCCESS;
}


