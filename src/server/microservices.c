#include "microservices.h"

typedef struct weather_ctx {
  double humidity;
  double pressure;
  double temperature;
} weather_ctx_t;

typedef struct currency_ctx {
  double value;
} currency_ctx_t;

/**
 * @brief Allocates the context for a microsever of type weather.
 *
 * @param pointer to server structure.
 */
void _create_weather_context(server_t* serv) {
  weather_ctx_t* context = malloc(sizeof(weather_ctx_t));
  if (context == NULL) {
    perror("Failed micro-context allocation!");
    return;
  }
  serv->context = context;
  context->humidity = 83;
  context->pressure = 10130;
  context->temperature = 29.3;
}

/**
 * @brief Allocates the context for a microsever of type currency.
 *
 * @param pointer to server structure.
 */
void _create_currency_context(server_t* serv) {
  currency_ctx_t* context = malloc(sizeof(currency_ctx_t));
  if (context == NULL) {
    perror("Failed micro-context allocation!");
    return;
  }
  serv->context = context;
  context->value = 1.99;
}
/**
 * @brief Handle the weather micro service request.
 *
 * @param response and request structures, along with server structure.
 */
static void _handle_weather(response_t *resp, const request_t *r, const server_t* serv) {
  printf("Request: weather\n");
  printf(" - city: %s\n", str_to_cstr(&r->u.weather.city));
  //TODO: send weather response according to city
  weather_ctx_t* context = (weather_ctx_t*) serv->context;
  resp->type = response_weather;
  resp->u.weather.humidity = context->humidity;
  resp->u.weather.pressure = context->pressure;
  resp->u.weather.temperature = context->temperature;
}
/**
 * @brief Handle the currency micro service request.
 *
 * @param response and request structures, along with server structure.
 */
static void _handle_currency(response_t *resp, const request_t *r, const server_t* serv) {
  printf("Request: currency\n");
  printf(" - currency: %s\n", str_to_cstr(&r->u.currency.currency));
  //TODO: send currency response according to currency
  currency_ctx_t* context = (currency_ctx_t*) serv->context;
  resp->type = response_currency;
  resp->u.currency.quote = context->value;
}
/**
 * @brief Handle the micro service request. 
 *
 * @param response and request structures.
 */
static void _micro_handle_request(response_t *resp, const request_t *r, const server_t* serv) {

  // A valid question is: why define the server's type if we still
  // have to switch() to get the corresponding behavior?
  // Well, while it is true, it doesn't look like a true microservice
  // if a process can handle any request.
  // Furthermore, in the future we might add more context to the server struct
  // (pending discussion on PR #9)

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
  
  if (type == request_weather) {
    _create_weather_context(&microserver);
  } else if(type == request_currency) {
    _create_currency_context(&microserver);
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
