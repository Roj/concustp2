/* include area */
#include "server.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SELF_PORT 8002

/** Flag that indicates the program should finish */
static bool exit_flag = false;

/**
 * @brief Handles exit signals.
 *
 * @param signal
 */
void sigint_handler(int signal) {
  /* signals the main loop to exit */
  exit_flag = true;
}

/**
 * @brief Request handler.
 *
 * @param s Client socket.
 */
static void _handle_request(response_t *resp, const request_t *r) {
  // TODO: handle request properly

  // Shouldn't this be delegated on the microservices??
  /* Prop: Use a hash in order to save all pairs (city, weather) or
   * (coin, value) */
  printf("request:\n");
  printf(" - type: %s\n", (r->type == request_weather ? "Weather" : "Currency"));
  switch (r->type) {
    case request_weather:
      printf(" - city: %s\n", str_to_cstr(&r->u.weather.city));
      break;
    case request_currency:
      printf(" - currency: %s\n", str_to_cstr(&r->u.currency.currency));
      break;
    case request_last:
      return;
  }

  // TODO: send the corresponding response
  resp->type = response_currency;
  resp->u.currency.quote = 1.123;
}

int main(int argc, const char *argv[]) {
  /* signal handling */
  signal(SIGINT, sigint_handler);

  printf("Starting server...\n");

  server_t server;
  if (!server_init(&server, SELF_PORT, _handle_request)) {
    perror("Error al iniciar servidor");
    return 1;
  }

  printf("server started!\n");

  /* handles client requests */
  while (!exit_flag) {
    printf("waiting connection...\n");
    if (!server_handle_request(&server)) {
      break;
    }
  }

  server_stop(&server);
  return 0;
}
