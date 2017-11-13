/* include area */
#include "server.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
static void _handle_request(const request_t *r, client_conn_t *client) {
  // TODO: handle request
  printf("request:\n");
  printf(" - type: %d\n", r->type);
  switch (r->type) {
    case req_weather:
      printf(" - city: %s\n", str_to_cstr(&r->u.weather.city));
      break;
    case req_currency:
      printf(" - currency: %s\n", str_to_cstr(&r->u.currency.currency));
      break;
  }
}

int main(int argc, const char *argv[]) {
  /* signal handling */
  signal(SIGINT, sigint_handler);

  printf("Starting server...\n");

  server_t server;
  if (!server_init(&server, 8002, _handle_request)) {
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
