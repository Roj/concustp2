#include "microservices.h"

/**
 * @brief Handle the weather micro service request.
 *
 * @param response and request structures, along with server structure.
 */
static void _handle_weather(response_t *resp, const request_t *r, const server_t* serv) {
  printf("Request: weather\n");
  printf(" - city: %s\n", str_to_cstr(&r->u.weather.city));
  //TODO: send weather response 
  resp->type = response_currency;
  resp->u.currency.quote = 1.123;
}
/**
 * @brief Handle the currency micro service request.
 *
 * @param response and request structures, along with server structure.
 */
static void _handle_currency(response_t *resp, const request_t *r, const server_t* serv) {
  printf("Request: currency\n");
  printf(" - currency: %s\n", str_to_cstr(&r->u.currency.currency));
  resp->type = response_currency;
  resp->u.currency.quote = 1.123;
}
/**
 * @brief Handle the micro service request. 
 *
 * @param response and request structures.
 */
static void _micro_handle_request(response_t *resp, const request_t *r, const server_t* serv) {

  //A valid question is: why define the server's type if we still
  //have to switch() to get the corresponding behavior?
  //Well, while it is true, it doesn't look like a true microservice
  //if a process can handle any request.

  if (r->type != serv->type && r->type != request_last) {
    perror("Sent request to wrong server!");
    return;
  }
  
  /* Prop: Use a hash in order to save all pairs (city, weather) or
   * (coin, value) */

  switch (serv->type) {
    case request_weather:
      _handle_weather(resp, r, serv);
      break;
    case request_currency:
      _handle_currency(resp, r, serv);
      break;
    case request_last:
      break;
  }

}
/**
 * @brief Launches and executes the main loop of the microservice.
 *
 * @param type of microservice and pointer to the exit flag that is
 * modified on signal (server/main.c:sigint_handler).
 */
int launch_microservice(request_type_t type, bool* exit_flag) {  
  server_t microserver;

  microserver.type = type;
  int port = SELF_PORT + type + 1; //Add +1, since enums start at 0.

  if (!server_init(&microserver, port, _micro_handle_request)) {
    perror("Error al iniciar el microservicio");
    return 1;
  }

  /* handles client (portal, middleware's) requests */
  while (!*exit_flag) {
    printf("waiting connection...\n");
    if (!server_handle_request(&microserver)) {
      break;
    }
  }

  server_stop(&microserver);
  return 0;
}
