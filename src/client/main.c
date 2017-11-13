/* include area */
#include <errno.h>
#include <netdb.h>
#include <socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, const char *argv[]) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("socket error");
    return 1;
  }

  struct sockaddr_in serv_addr = {0};
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(8002);

  struct hostent *server = gethostbyname("localhost");
  if (server == NULL) {
    perror("ghbn");
    close(fd);
    return 1;
  }

  memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

  int connectOk = connect(fd, ( const struct sockaddr * )&serv_addr, sizeof(serv_addr));
  if (connectOk < 0) {
    perror("connect error");
    close(fd);
    return 1;
  }

  /* writes a hardcoded request */
  const char *data = "\x01"
                     "0007"
                     "dollars";
  write(fd, data, strlen(data));

  return 0;
}
