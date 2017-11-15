/* include area */
#include "client.h"
#include "str.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT 8002

int main(int argc, const char *argv[]) {
  /* initializes the request */
  request_t req = {0};
  req.type = request_currency;
  str_init(&req.u.currency.currency, "pesos");

  response_t resp = {0};
  if (!client_send(&resp, SERVER_PORT, &req)) {
    perror("Failed sending the request");
    return 1;
  }

  // TODO: handle all responses
  switch (resp.type) {
    case response_currency:
      printf("Currency response: %f\n", resp.u.currency.quote);
      break;
    default:
      break;
  }

  return 0;
}
