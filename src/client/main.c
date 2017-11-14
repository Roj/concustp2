/* include area */
#include "client.h"
#include "str.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, const char *argv[]) {
  /* initializes the request */
  request_t req = {0};
  req.type = req_currency;
  str_init(&req.u.currency.currency, "pesos");

  response_t resp = {0};
  if (!client_send(&resp, 8002, &req)) {
    perror("Failed sending the request");
    return 1;
  }

  // TODO: handle all responses
  switch (resp.type) {
    case resp_currency:
      printf("Currency response: %f\n", resp.u.currency.quote);
      break;
    default:
      break;
  }

  return 0;
}
