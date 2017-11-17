/* include area */
#include "server.h"
#include "client.h"
#include "microservices.h"
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
static void _handle_request(response_t *resp, const request_t *r) {
  //Send request to relevant microservice.
  //XXX: Question: should the middleware do more than that?
  if (!client_send(resp, SELF_PORT + r->type + 1, r)) {
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

  int fork_weather = fork();
  if (fork_weather < 0) {
    perror("Error al iniciar el servicio del clima");
    return 2;
  }

  if (fork_weather == 0)
    return launch_microservice(request_weather, &exit_flag);

  int fork_currency = fork();
  
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
