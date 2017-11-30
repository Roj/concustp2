#ifndef SERVER_H
#define SERVER_H

/* include area */
#include "requests.h"
#include <netdb.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MAX_PENDING_CONN 100
#define SELF_PORT 8002

typedef struct server server_t;
/**
 * Server request handler
 * This callback is executed *in an independent process* whenever a
 * client sends a request to the server.
 *
 * req contains the (parsed) client's request.
 * resp is to be filled by inside the callback with the corresponding data
 *      and after the callback is finished, it's serialized and sent to
 *      client.
 */
typedef void (*req_handler_t)(response_t *resp, const request_t *req, const struct server *serv);

/** Server type */
struct server {
  int fd;
  struct sockaddr_in serv_addr;
  struct sockaddr_in cli_addr;
  req_handler_t handler;
  request_type_t type; // Used in microservices. Ignored in middleware.
  void *context;       // Optional. Aids microservices to hold state.
};

/*-------------------------------------------------------------------------
  Server
-------------------------------------------------------------------------*/

bool server_init(server_t *s, uint16_t port, req_handler_t handler);
bool server_handle_request(server_t *s);
void server_stop(server_t *s);

#endif
