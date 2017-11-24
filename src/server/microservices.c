#include "microservices.h"
#include <jansson.h>
#define WEATHER_JSON_FILE "weather.json"
#define CURRENCY_JSON_FILE "currency.json"

typedef struct weather_ctx {
  json_t* json;
} weather_ctx_t;

typedef struct currency_ctx {
  json_t* json;
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
  json_error_t json_load_error;
  json_t* weather_json = json_load_file(WEATHER_JSON_FILE, 0, &json_load_error);
  if (weather_json == NULL) {
    perror("Failed JSON load of weather file!");
    return;
  }
  context->json = weather_json;
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
  json_error_t json_load_error;
  json_t* currency_json = json_load_file(CURRENCY_JSON_FILE, 0, &json_load_error);
  if (currency_json == NULL) {
    perror("Failed JSON load of currency file!");
    return;
  }
  context->json = currency_json;
}
/**
 * @brief Fills a response with the weather status for a given city.
 *
 * @param weather context of server (w/ structures), city name and struct* weather response.
 */
void _get_city_weather(weather_ctx_t* context, const string_t* city, response_weather_t* resp) {
  json_t* weather_json = json_object_get(context->json, str_to_cstr(city));
  if (weather_json == NULL) {
    perror("Error trying to fetch weather info for city");
    return;
  }
  //This doesn't lose memory because of borrowed references.
  //https://jansson.readthedocs.io/en/2.10/apiref.html#c.json_decref
  resp->humidity = json_integer_value(json_object_get(weather_json, "humidity"));
  resp->pressure = json_real_value(json_object_get(weather_json, "pressure"));
  resp->temperature = json_real_value(json_object_get(weather_json, "temperature"));
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
  _get_city_weather(context, &r->u.weather.city, &resp->u.weather);
}

/**
 * @brief Obtains the exchange value for a given currency.
 *
 * @param currency context of server (w/ structures) and currency name.
 */
double _get_currency_exchange(currency_ctx_t* context, const string_t* currency) {
  return json_real_value(json_object_get(context->json, str_to_cstr(currency)));
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
  resp->u.currency.quote = _get_currency_exchange(context, &r->u.currency.currency);
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
