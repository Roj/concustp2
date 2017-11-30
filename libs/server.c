/* include area */
#include "server.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

/**
 * @brief Reads from a socket.
 *
 * @param output Buffer where the read data is copied to.
 * @param bytes Buffer size.
 * @param cb_ctx User defined callback context.
 * @return size_t Bytes read.
 */
static size_t _socket_read(void *output, size_t bytes, void *cb_ctx) {
  int *fd = cb_ctx;

  ssize_t bytes_read = 0;
  printf("reading %zu\n", bytes);
  do {
    bytes_read += recv(*fd, ( char * )output + bytes_read, bytes - bytes_read, 0);
    printf("read %d %*s\n", ( int )bytes_read, ( int )bytes_read, ( char * )output);
  } while (errno == EINTR);

  /* must have read the exact number of bytes requested*/
  return bytes_read;
}

/**
 * @brief Response serialization callback that outputs through a socket.
 *
 * @param data Data to write.
 * @param bytes Numer of bytes to write.
 * @param cb_ctx Client connection.
 * @return false on error, true on success.
 */
static bool _socket_write(const void *data, size_t bytes, void *cb_ctx) {
  int *fd = cb_ctx;

  ssize_t bytes_sent = 0;
  do {
    bytes_sent = send(*fd, data, bytes, MSG_NOSIGNAL);
  } while (errno == EINTR);

  /* must have sent the all the data */
  return (bytes_sent == bytes);
}

/**
 * @brief Handles a client connection.
 *
 * @param s The server.
 * @param client_fd The file descriptor of the connected client.
 * @return false The server should stop receiving requests, true to continue.
 */
static bool _on_request(server_t *s, int client_fd) {
  
  /* creates a new process to handle the request */

  /* deserializes the request */
  request_t req = {0};
  if (!request_deserialize(&req, _socket_read, &client_fd))
    return false;

  /* calls the handler */
  response_t resp = {0};
  s->handler(&resp, &req, s);

  /* sends the response */
  if (!response_serialize(&resp, _socket_write, &client_fd)) {
    perror("Failed sending the response");
  }

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
  int listenOk = listen(s->fd, MAX_PENDING_CONN);
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
  _on_request(s, client);

  /* closes the connection */
  close(client);

  return true;
}

/**
 * @brief Stops a server that was previously started.
 *
 * @param s Server to stop.
 * @return false
 */
void server_stop(server_t *s) {
  /* waits until all requests are handled (i.e. all children finished) */
  do {
    int status;
    wait(&status);
  } while (errno != ECHILD);

  close(s->fd);
}
