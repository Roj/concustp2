/* include area */
#include "client.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Reads from a socket.
 *
 * @param output Output buffer.
 * @param bytes Number of bytes to read.
 * @param cb_ctx Callback context.
 * @return false on error, true on success.
 */
static bool _socket_read(void *output, size_t bytes, void *cb_ctx) {
  int *fd = cb_ctx;

  ssize_t bytes_read = 0;
  do {
    bytes_read += recv(*fd, ( char * )output + bytes_read, bytes - bytes_read, MSG_WAITALL);
  } while (errno == EINTR);

  /* must have read the exact number of bytes requested*/
  return (bytes_read == bytes);
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
 * @brief Sends a requests to the given port and waits for the response.
 *
 * @param resp Response to the request (output).
 * @param port Port where the request is sent to.
 * @param req Request that will be sent (properly initialized by the caller).
 * @return false on error, true on success.
 */
bool client_send(response_t *resp, uint16_t port, const request_t *req) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("client - socket error");
    /* I'd rather use the log_write function in order to have a logfile
    for each different process with specific failure info */
    return false;
  }

  struct sockaddr_in serv_addr = {0};
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  struct hostent *server = gethostbyname("localhost");
  if (server == NULL) {
    perror("client - gethostbyname");
    close(fd);
    return false;
  }

  memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

  int connectOk = connect(fd, ( const struct sockaddr * )&serv_addr, sizeof(serv_addr));
  if (connectOk < 0) {
    perror("client connect error");
    close(fd);
    return false;
  }

  /* sends the request and waits the response */
  bool success = request_serialize(req, _socket_write, &fd);
  if (success) {
    success = response_deserialize(resp, _socket_read, &fd);
  }

  /* cleanup */
  close(fd);

  return success;
}
