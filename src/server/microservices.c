#include "microservices.h"

/**
 * @brief Handle the micro service request. 
 *
 * @param response and request structures.
 */
static void _micro_handle_request(response_t *resp, const request_t *r) {
  
  // TODO: handle request properly

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
/**
 * @brief Launches and executes the main loop of the microservice.
 *
 * @param type of microservice and pointer to the exit flag that is
 * modified on signal (server/main.c:sigint_handler).
 */
int launch_microservice(request_type_t type, bool* exit_flag) {  
  server_t microserver;

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
