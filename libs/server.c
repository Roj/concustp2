/* include area */
#include "server.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

static bool _socket_read(void *output, size_t bytes, void *cb_ctx) {
  int *fd = cb_ctx;
  return (read(*fd, output, bytes) == bytes);
}

static bool _on_request(server_t *s, int client_fd) {
  /* creates a new process to handle the request */
  pid_t pid = fork( );
  if (pid < 0) {
    return false;
  }

  /* checks if it's the parent process */
  if (pid > 0) {
    /* lest's the child handle the request, so does nothing */
    return true;
  }

  /* deserializes the request */
  request_t r;
  if (!request_deserialize(&r, _socket_read, &client_fd))
    return false;

  /* calls the handler */
  client_conn_t conn = {.fd = client_fd};
  s->handler(&r, &conn);

  /* it's a child process, so it should stop the server */
  return false;
}

/**
 * @brief Initializes the server.
 *
 * @param s Server to initialize.
 * @param port Port where the server waits for connections.
 * @param handler Request handler.
 * @return false in case of error, true otherwise.
 */
bool server_init(server_t *s, uint16_t port, req_handler_t handler) {
  s->fd = socket(AF_INET, SOCK_STREAM, 0);
  if (s->fd < 0) {
    perror("socket");
    return false;
  }

  memset(&s->serv_addr, 0, sizeof(s->serv_addr));
  s->serv_addr.sin_family = AF_INET;
  s->serv_addr.sin_port = htons(port);
  s->serv_addr.sin_addr.s_addr = INADDR_ANY;

  /* binds the socket */
  int bindOk = bind(s->fd, ( struct sockaddr * )&(s->serv_addr), sizeof(s->serv_addr));
  if (bindOk < 0) {
    perror("bind");
    close(s->fd);
    return false;
  }

  /* marks the socket as passive */
  int listenOk = listen(s->fd, 100);
  if (listenOk < 0) {
    perror("listen");
    close(s->fd);
    return false;
  }

  s->handler = handler;

  return true;
}

/**
 * @brief Waits for a client to connect and handles the request.
 *
 * @return false on error, true otherwise.
 */
bool server_handle_request(server_t *s) {
  size_t addr_size = sizeof(s->cli_addr);

  /* waits for a client to connect */
  int client = accept(s->fd, ( struct sockaddr * )&s->cli_addr, ( socklen_t * )&addr_size);
  if (client < 0) {
    perror("accept");
    return false;
  }

  /* handles the request */
  bool rv = _on_request(s, client);

  /* closes the connection */
  close(client);

  return rv;
}

/**
 * @brief Response serialization callback that outputs through a socket.
 *
 * @param data Data to write.
 * @param bytes Numer of bytes to write.
 * @param cb_ctx Client connection.
 * @return false on error, true on success.
 */
static bool _conn_write(const void *data, size_t bytes, void *cb_ctx) {
  client_conn_t *c = cb_ctx;
  return (write(c->fd, data, bytes) == bytes);
}

/**
 * @brief Stops a server that was previously started.
 *
 * @param s Server to stop.
 * @return false
 */
void server_stop(server_t *s) {
  /* waits until all requests are handled (i.e. all childen finished) */
  int wait_rv;
  do {
    int status;
    wait_rv = wait(&status);
  } while (wait_rv != ECHILD);

  close(s->fd);
}

/**
 * @brief Sends a response to the client.
 *
 * @param c Client connection.
 * @param r Response (must be properly filled).
 * @return false on error, true on success.
 */
bool client_conn_write(client_conn_t *c, const response_t *r) {
  return response_serialize(r, _conn_write, c);
}
