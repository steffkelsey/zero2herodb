#ifndef SRVPOLL_H
#define SRVPOLL_H

#include "parse.h"
#include <poll.h>

#define MAX_CLIENTS 256
#define PORT 8080
#define BUFF_SIZE 4096

typedef enum {
  STATE_NEW,
  STATE_CONNECTED,
  STATE_DISCONNECTED,
  STATE_HELLO,
  STATE_MSG,
  STATE_GOODBYE
} state_e;

// Structure to hold client state
typedef struct {
  int fd;
  state_e state;
  char buffer[4096];
} clientstate_t;

void init_clients(clientstate_t *states);
int find_free_slot(clientstate_t *states);
int find_slot_by_fd(clientstate_t *states, int fd);
void handle_client_fsm(struct dbheader_t *dbhdr, struct employee_t **employees, clientstate_t *client, int dbfd);

void fsm_general_reply(clientstate_t* const client, dbproto_hdr_t* const hdr, const dbproto_type_e type);
inline void fsm_reply_hello_err(clientstate_t* const client, dbproto_hdr_t* const hdr) { fsm_general_reply(client, hdr, MSG_ERROR); }

inline void fsm_reply_add(clientstate_t* const client, dbproto_hdr_t* const hdr) { fsm_general_reply(client, hdr, MSG_EMPLOYEE_ADD_RES); }
inline void fsm_reply_add_err(clientstate_t* const client, dbproto_hdr_t* const hdr) { fsm_general_reply(client, hdr, MSG_ERROR); }

inline void fsm_reply_del(clientstate_t* const client, dbproto_hdr_t* const hdr) { fsm_general_reply(client, hdr, MSG_EMPLOYEE_DEL_RES); }
inline void fsm_reply_del_err(clientstate_t* const client, dbproto_hdr_t* const hdr) { fsm_general_reply(client, hdr, MSG_ERROR); }

inline void fsm_reply_missing_err(clientstate_t* const client, dbproto_hdr_t* const hdr) { fsm_general_reply(client, hdr, MSG_EMPLOYEE_MISSING_RES); }

#endif
