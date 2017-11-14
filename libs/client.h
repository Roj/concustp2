#ifndef CLIENT_H
#define CLIENT_H

/* include area */
#include "requests.h"
#include <netdb.h>
#include <socket.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

/*-------------------------------------------------------------------------
  Client
-------------------------------------------------------------------------*/

bool client_send(response_t *resp, uint16_t port, const request_t *req);

#endif
