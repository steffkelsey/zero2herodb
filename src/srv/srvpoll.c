#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <poll.h>
#include <string.h>

#include "common.h"
#include "srvpoll.h"

void fsm_general_reply(clientstate_t* const client, dbproto_hdr_t* const hdr, const dbproto_type_e type)
{
    hdr->type = htonl(type);
    hdr->len = htons(0); // sending 0 aditional elements

    write(client->fd, hdr, sizeof(dbproto_hdr_t));
}

void fsm_reply_hello(clientstate_t *client, dbproto_hdr_t *hdr) {
  hdr->type = MSG_HELLO_RES;
  hdr->len = 1;

  dbproto_hello_resp* hello = (dbproto_hello_resp*)&hdr[1];
  hello->proto = PROTO_VER;

  hdr->type = htonl(hdr->type);
  hdr->len = htons(hdr->len);
  hello->proto = htons(hello->proto);

  // write the hello message response
  write(client->fd, hdr, sizeof(dbproto_hdr_t) + sizeof(dbproto_hello_resp));
}

extern inline void fsm_reply_err(clientstate_t* const client, dbproto_hdr_t* const hdr);
extern inline void fsm_reply_hello_err(clientstate_t* const client, dbproto_hdr_t* const hdr);
extern inline void fsm_reply_missing_err(clientstate_t* const client, dbproto_hdr_t* const hdr);

extern inline void fsm_reply_add(clientstate_t* const client, dbproto_hdr_t* const hdr);
extern inline void fsm_reply_add_err(clientstate_t* const client, dbproto_hdr_t* const hdr);

extern inline void fsm_reply_upd(clientstate_t* const client, dbproto_hdr_t* const hdr);
extern inline void fsm_reply_upd_err(clientstate_t* const client, dbproto_hdr_t* const hdr);

extern inline void fsm_reply_del(clientstate_t* const client, dbproto_hdr_t* const hdr);
extern inline void fsm_reply_del_err(clientstate_t* const client, dbproto_hdr_t* const hdr);

void send_employees(struct dbheader_t *dbhdr, struct employee_t *employees, clientstate_t *client) {
  printf("sending employees...\n");
  dbproto_hdr_t *hdr = (dbproto_hdr_t*)client->buffer;
  hdr->type = htonl(MSG_EMPLOYEE_LIST_RES);
  hdr->len = htons(dbhdr->count);

  ssize_t bytes_written = write(client->fd, hdr, sizeof(dbproto_hdr_t));
  if (bytes_written == -1) {
    perror("Unable to write from header to client->fd");
    printf("error 1\n");
    return;
  }
  if (bytes_written != sizeof(dbproto_hdr_t)) {
    printf("error 2\n");
    printf("Partial write. expected %zu bytes, but wrote %zd bytes\n", sizeof(dbproto_hdr_t), bytes_written);
    return;
  }

  dbproto_employee_list_resp *employee = (dbproto_employee_list_resp*)&hdr[1];

  int i = 0;
  for (; i < dbhdr->count; i++) {
    strncpy(employee->name, employees[i].name, sizeof(employee->name));
    strncpy(employee->address, employees[i].address, sizeof(employee->address));
    employee->hours = htonl(employees[i].hours);
    printf("sending employee[%d]: %s\n", i, employees[i].name);
    write(client->fd, employee, sizeof(dbproto_employee_list_resp));
  }
}

void handle_client_fsm(struct dbheader_t *dbhdr, struct employee_t **employees, clientstate_t *client, int dbfd) {
  printf("handling client FSM....\n");
  dbproto_hdr_t *hdr = (dbproto_hdr_t*)client->buffer;

  hdr->type = ntohl(hdr->type);
  hdr->len = ntohs(hdr->len);

  if (client->state == STATE_HELLO) {
    printf("client->state is STATE_HELLO\n");
    if (hdr->type != MSG_HELLO_REQ || hdr->len != 1) {
      printf("didn't get MSG_HELLO in HELLO state...\n");
      // TODO send err message
    }

    dbproto_hello_req* hello = (dbproto_hello_req*)&hdr[1];
    hello->proto = ntohs(hello->proto);
    if (hello->proto != PROTO_VER) {
      printf("Protocol mismatch...\n");
      fsm_reply_hello_err(client, hdr);
      return;
    }
    
    fsm_reply_hello(client, hdr);
    client->state = STATE_MSG;
    printf("Client upgraded to STATE_MSG\n");
  }

  if (client->state == STATE_MSG) {
    if (hdr->type == MSG_EMPLOYEE_ADD_REQ) {
      printf("MSG_EMPLOYEE_ADD_REQ\n");
      dbproto_employee_add_req* employee = (dbproto_employee_add_req*)&hdr[1];
      
      printf("Adding employee: %s\n", employee->data);
      if (add_employee(dbhdr, employees, employee->data) != STATUS_SUCCESS) {
        printf("error adding employee!\n");
        fsm_reply_add_err(client, hdr);
        return;        
      } else {
        fsm_reply_add(client, hdr);
        if (output_file(dbfd, dbhdr, *employees) != STATUS_SUCCESS) {
          printf("error writing output file\n");
        } else {
          printf("Success!\n");
          list_employees(dbhdr, *employees);
        }
      }
    }
    if (hdr->type == MSG_EMPLOYEE_LIST_REQ) {
      printf("MSG_EMPLOYEE_LIST_REQ\n");
      send_employees(dbhdr, *employees, client);      
    } 
    if (hdr->type == MSG_EMPLOYEE_DEL_REQ) {
      printf("MSG_EMPLOYEE_DEL_REQ\n");
      // get the employee data from the delete request
      dbproto_employee_del_req* employee = (dbproto_employee_del_req*)&hdr[1];
      printf("Deleting employee: %s\n", employee->data);
      if (remove_employee(dbhdr, employees, employee->data) != STATUS_SUCCESS) {
        printf("error deleting employee!\n");
        fsm_reply_del_err(client, hdr);
        return;        
      } else {
        fsm_reply_del(client, hdr);
        if (output_file(dbfd, dbhdr, *employees) != STATUS_SUCCESS) {
          printf("error writing output file\n");
        } else {
          printf("Success!\n");
          list_employees(dbhdr, *employees);
        }
      }
    } 
    if (hdr->type == MSG_EMPLOYEE_UPD_REQ) {
      printf("MSG_EMPLOYEE_UPD_REQ\n");
      // get the employee data from the update request
      dbproto_employee_upd_req* employee = (dbproto_employee_upd_req*)&hdr[1];
      printf("Updating employee: %s\n", employee->data);
      if (update_employee(dbhdr, *employees, employee->data) != STATUS_SUCCESS) {
        printf("error updating employee!\n");
        fsm_reply_upd_err(client, hdr);
        return;        
      } else {
        fsm_reply_upd(client, hdr);
        if (output_file(dbfd, dbhdr, *employees) != STATUS_SUCCESS) {
          printf("error writing output file\n");
        } else {
          printf("Success!\n");
          list_employees(dbhdr, *employees);
        }
      }
    }
  }
}

void init_clients(clientstate_t *states) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    states[i].fd = -1; // -1 indicates  a free slot
    states[i].state = STATE_NEW;
    memset(&states[i].buffer, '\0', BUFF_SIZE);
  }
}

int find_free_slot(clientstate_t *states) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (states[i].fd == -1) {
      return i;
    }
  }
  return -1; //no free slot found
}

int find_slot_by_fd(clientstate_t *states, int fd) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (states[i].fd == fd) {
      return i;
    }
  }
  return -1; // Not found
}
