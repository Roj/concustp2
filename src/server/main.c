/* include area */
#include "server.h"
#include "client.h"
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
  //Send request to relevant microservice.
  //XXX: Question: should the middleware do more than that?
  if (!client_send(resp, SELF_PORT+r->type, r)) {
    perror("Error sending the request to the microservice");
    return;
  }

}

static void _micro_handle_request(response_t *resp, const request_t *r) {
  
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

int launch_microservice(request_type_t type) {  
  server_t microserver;

  int port = SELF_PORT + type + 1; //Add +1, since enums start at 0.
  if (!server_init(&microserver, port, _micro_handle_request)) {
    perror("Error al iniciar el microservicio");
    return 1;
  }

  /* handles client (portal, middleware's) requests */
  while (!exit_flag) {
    printf("waiting connection...\n");
    if (!server_handle_request(&microserver)) {
      break;
    }
  }

  server_stop(&microserver);
  return 0;
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
  printf("Launching microservices..");

  int fork_weather = fork();
  if (fork_weather < 0) {
    perror("Error al iniciar el servicio del clima");
    return 2;
  }

  if (fork_weather == 0)
    return launch_microservice(request_weather);

  int fork_currency = fork();
  
  if (fork_currency < 0) {
    perror("Error al iniciar el servicio de divisas");
    return 2;
  }
  
  if (fork_currency == 0)
    return launch_microservice(request_currency);

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
