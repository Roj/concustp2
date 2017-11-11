#ifndef SOCKET_H
#define SOCKET_H

/* include area */
#include "sys/socket.h"
#include "sys/types.h"

/** socket type */
typedef struct {
  /** socket file descriptor */
  int fd;

} socket_t;

#endif
