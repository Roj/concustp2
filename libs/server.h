#ifndef SERVER_H
#define SERVER_H

/* include area */
#include "requests.h"
#include <netdb.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>

/** Connection used to send the response to the clients */
typedef struct { int fd; } client_conn_t;

/** Server request handler */
typedef void (*req_handler_t)(response_t *resp, const request_t *req);

/** Server type */
typedef struct {
  int fd;
  struct sockaddr_in serv_addr;
  struct sockaddr_in cli_addr;
  req_handler_t handler;
} server_t;

/*-------------------------------------------------------------------------
  Server
-------------------------------------------------------------------------*/

bool server_init(server_t *s, uint16_t port, req_handler_t handler);
bool server_handle_request(server_t *s);
void server_stop(server_t *s);

#endif
