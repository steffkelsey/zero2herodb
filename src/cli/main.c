#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

#include "common.h"

int send_employee_list(int fd) {
  char buf[4096] = {0};

  dbproto_hdr_t *hdr = (dbproto_hdr_t*)buf;
  hdr->type = MSG_EMPLOYEE_LIST_REQ;
  hdr->len = 0;

  // pack for network
  hdr->type = htonl(hdr->type);
  hdr->len = htons(hdr->len);

  // write the request message
  write(fd, buf, sizeof(dbproto_hdr_t));

  // recv the response
  read(fd, buf, sizeof(buf));

  hdr->type = ntohl(hdr->type);
  hdr->len = ntohs(hdr->len);

  // handle error messages
  if (hdr->type == MSG_ERROR) {
    printf("Unable to list employees.\n");
    close(fd);
    return STATUS_ERROR;
  }

  // handle employee list response
  if (hdr->type == MSG_EMPLOYEE_LIST_RES) {
    dbproto_employee_list_resp *employee = (dbproto_employee_list_resp*)&hdr[1];

    int i = 0;
    for (; i < hdr->len; i++) {
      read(fd, employee, sizeof(dbproto_employee_list_resp));
      employee->hours = ntohl(employee->hours);
      printf("%s, %s, %d\n", employee->name, employee->address, employee->hours);
    }
  }
  
  return STATUS_SUCCESS;
}

int send_employee_add(int fd, char *addstr) {
  char buf[4096] = {0};

  dbproto_hdr_t *hdr = (dbproto_hdr_t*)buf;
  hdr->type = MSG_EMPLOYEE_ADD_REQ;
  hdr->len = 1;

  // Send the add employee request with the data
  dbproto_employee_add_req* employee = (dbproto_employee_add_req*)&hdr[1];
  strncpy(employee->data, addstr, sizeof(employee->data));

  hdr->type = htonl(hdr->type);
  hdr->len = htons(hdr->len);

  // write the message
  write(fd, buf, sizeof(dbproto_hdr_t) + sizeof(dbproto_employee_add_req));

  // recv the response
  read(fd, buf, sizeof(buf));

  hdr->type = ntohl(hdr->type);
  hdr->len = ntohs(hdr->len);

  // handle error messages
  if (hdr->type == MSG_ERROR) {
    printf("Improper format for add employee string.\n");
    close(fd);
    return STATUS_ERROR;
  }
  
  // return success
  printf("Employee added.\n");
  return STATUS_SUCCESS;
}

int send_employee_update(int fd, char *addstr) {
  char buf[4096] = {0};

  dbproto_hdr_t *hdr = (dbproto_hdr_t*)buf;
  hdr->type = MSG_EMPLOYEE_UPD_REQ;
  hdr->len = 1;

  // Send the add employee request with the data
  dbproto_employee_upd_req* employee = (dbproto_employee_upd_req*)&hdr[1];
  strncpy(employee->data, addstr, sizeof(employee->data));

  hdr->type = htonl(hdr->type);
  hdr->len = htons(hdr->len);

  // write the message
  write(fd, buf, sizeof(dbproto_hdr_t) + sizeof(dbproto_employee_upd_req));

  // recv the response
  read(fd, buf, sizeof(buf));

  hdr->type = ntohl(hdr->type);
  hdr->len = ntohs(hdr->len);

  // handle error messages
  if (hdr->type == MSG_ERROR) {
    printf("Improper format for update employee string.\n");
    close(fd);
    return STATUS_ERROR;
  }
  
  // return success
  printf("Employee updated.\n");
  return STATUS_SUCCESS;
}

int send_employee_remove(int fd, char *delstr) {
    char buf[4096] = {0};

  dbproto_hdr_t *hdr = (dbproto_hdr_t*)buf;
  hdr->type = MSG_EMPLOYEE_DEL_REQ;
  hdr->len = 1;

  // Send the del employee request with the data
  dbproto_employee_del_req* employee = (dbproto_employee_del_req*)&hdr[1];
  strncpy(employee->data, delstr, sizeof(employee->data));

  hdr->type = htonl(hdr->type);
  hdr->len = htons(hdr->len);

  // write the request message
  write(fd, buf, sizeof(dbproto_hdr_t) + sizeof(dbproto_employee_del_req));

  // recv the response
  read(fd, buf, sizeof(buf));

  hdr->type = ntohl(hdr->type);
  hdr->len = ntohs(hdr->len);

  // handle error messages
  if (hdr->type == MSG_ERROR) {
    printf("Improper format for delete employee string.\n");
    close(fd);
    return STATUS_ERROR;
  }
  
  // return success
  printf("Employee deleted.\n");
  return STATUS_SUCCESS;
}

int send_hello(int fd) {
  char buf[4096] = {0};

  dbproto_hdr_t *hdr = (dbproto_hdr_t*)buf;
  hdr->type = MSG_HELLO_REQ;
  hdr->len = 1;

  // Send the hello request with the version
  dbproto_hello_req* hello = (dbproto_hello_req*)&hdr[1];
  hello->proto = PROTO_VER;

  hdr->type = htonl(hdr->type);
  hdr->len = htons(hdr->len);
  hello->proto = htons(hello->proto);

  // write the hello message
  write(fd, buf, sizeof(dbproto_hdr_t) + sizeof(dbproto_hello_req));

  // recv the response
  read(fd, buf, sizeof(buf));

  hdr->type = ntohl(hdr->type);
  hdr->len = ntohs(hdr->len);

  // handle error messages
  if (hdr->type == MSG_ERROR) {
    printf("Protocol mismatch.\n");
    close(fd);
    return STATUS_ERROR;
  }
  
  // return success
  printf("Server connected, protocol v1.\n");
  return STATUS_SUCCESS;
}

int main (int argc, char *argv[]) {
  char *addarg = NULL;
  char *delarg = NULL;
  char *updarg = NULL;
  char *portarg = NULL, *hostarg = NULL;
  unsigned short port = 0;
  bool list = false;

  int c;
  while ((c = getopt(argc, argv, "lp:h:a:r:u:")) != -1) {
    switch (c) {
      case 'a':
        addarg = optarg;
        break;
      case 'p':
        portarg = optarg;
        port = atoi(portarg);
        break;
      case 'h':
        hostarg = optarg;
        break;
      case 'r':
        delarg = optarg;
        break;
      case 'u':
        updarg = optarg;
        break;
      case 'l':
        list = true;
        break;
      case '?':
        printf("Unknown option -%c\n", c);
        break;
      default:
        return -1;
    }    
  }

  if (port == 0) {
    printf("Bad port: %s\n", portarg);
    return -1;
  }

  if (hostarg == NULL) {
    printf("Must specify host with -h\n");
    return -1;
  }

  struct sockaddr_in serverInfo = {0};
  serverInfo.sin_family = AF_INET;
  serverInfo.sin_addr.s_addr = inet_addr(hostarg);
  serverInfo.sin_port = htons(port);
  
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    perror("socket");
    return -1;
  }

  if (connect(fd, (struct sockaddr*)&serverInfo, sizeof(serverInfo)) == -1) {
    perror("connect");
    close(fd);
    return -1;
  }

  if (send_hello(fd) != STATUS_SUCCESS) {
    // didn't close the fd here because the send_hello method closes it on error
    return -1;
  }

  if (addarg) {
    send_employee_add(fd, addarg);
  }

  if (updarg) {
    send_employee_update(fd, updarg);
  }

  if (delarg) {
    send_employee_remove(fd, delarg);
  }

  if (list) {
    send_employee_list(fd);
  }

  close(fd);
}
