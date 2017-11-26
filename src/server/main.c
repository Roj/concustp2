/* include area */
#include "server.h"
#include "client.h"
#include "microservices.h"
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
 * @brief Middleware request handler.
 *
 * @param response to be formed, request sent by client, and server struct entity.
 */
static void _handle_request(response_t *resp, const request_t *r, const server_t *serv) {
  // Send request to relevant microservice.
  int port = SELF_PORT + 1 + get_base_request(r->type);
  if (!client_send(resp, port, r)) {
    perror("Error sending the request to the microservice");
    return;
  }
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

  /* microservicios */
  printf("Launching microservices..\n");

  int fork_weather = fork( );
  if (fork_weather < 0) {
    perror("Error al iniciar el servicio del clima");
    return 2;
  }

  if (fork_weather == 0)
    return launch_microservice(request_weather, &exit_flag);

  int fork_currency = fork( );

  if (fork_currency < 0) {
    perror("Error al iniciar el servicio de divisas");
    return 2;
  }

  if (fork_currency == 0)
    return launch_microservice(request_currency, &exit_flag);

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
