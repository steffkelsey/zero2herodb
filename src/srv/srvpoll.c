#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <poll.h>

#include "common.h"
#include "srvpoll.h"

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

void fsm_reply_hello_err(clientstate_t *client, dbproto_hdr_t *hdr) {
  hdr->type = htonl(MSG_ERROR);
  hdr->len = htons(0);

  write(client->fd, hdr, sizeof(dbproto_hdr_t));
}

void fsm_reply_add(clientstate_t *client, dbproto_hdr_t *hdr) {
  printf("fsm_reply_add....\n");
  hdr->type = MSG_EMPLOYEE_ADD_RES;
  hdr->len = 0;

  hdr->type = htonl(hdr->type);
  hdr->len = htons(hdr->len);

  // write the hello message response
  write(client->fd, hdr, sizeof(dbproto_hdr_t));
}

void fsm_reply_add_err(clientstate_t *client, dbproto_hdr_t *hdr) {
  hdr->type = htonl(MSG_ERROR);
  hdr->len = htons(0);

  write(client->fd, hdr, sizeof(dbproto_hdr_t));
}

void handle_client_fsm(struct dbheader_t *dbhdr, struct employee_t *employees, clientstate_t *client, int dbfd) {
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
      dbproto_employee_add_req* employee = (dbproto_employee_add_req*)&hdr[1];
      
      printf("Adding employee: %s\n", employee->data);
      if (add_employee(dbhdr, &employees, employee->data) != STATUS_SUCCESS) {
        printf("error adding employee!\n");
        fsm_reply_add_err(client, hdr);
        return;        
      } else {
        fsm_reply_add(client, hdr);
        if (output_file(dbfd, dbhdr, employees) != STATUS_SUCCESS) {
          printf("error writing output file\n");
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
