#include "microservices.h"
#include <jansson.h>
#define WEATHER_JSON_FILE "weather.json"
#define CURRENCY_JSON_FILE "currency.json"
#define INVALID_VALUE -999
#define VALID(n) (n) != INVALID_VALUE

typedef struct weather_ctx { json_t *json; } weather_ctx_t;

typedef struct currency_ctx { json_t *json; } currency_ctx_t;

/**
 * @brief Allocates the context for a microsever of type weather.
 *
 * @param pointer to server structure.
 */
void _create_weather_context(server_t *serv) {
  weather_ctx_t *context = malloc(sizeof(weather_ctx_t));
  if (context == NULL) {
    perror("Failed micro-context allocation!");
    return;
  }
  serv->context = context;
  json_error_t json_load_error;
  json_t *weather_json = json_load_file(WEATHER_JSON_FILE, 0, &json_load_error);
  if (weather_json == NULL) {
    perror("Failed JSON load of weather file!");
    return;
  }
  context->json = weather_json;
}

/**
 * @brief Frees the resources associated to the weather microserv. Optionally writes the state to disk.
 *
 * @param pointer to server structure & bool to save state.
 */
void _finish_weather_service(server_t *serv, bool save_state) {
  weather_ctx_t *ctx = serv->context;
  if (json_dump_file(ctx->json, WEATHER_JSON_FILE, 0) == -1) {
    perror("Error saving weather state to file");
  }
  json_decref(ctx->json);
  free(ctx);
}

/**
 * @brief Frees the resources associated to the currency microserv. Optionally writes the state to disk.
 *
 * @param pointer to server structure & bool to save state.
 */
void _finish_currency_service(server_t *serv, bool save_state) {
  weather_ctx_t *ctx = serv->context;
  if (json_dump_file(ctx->json, CURRENCY_JSON_FILE, 0) == -1) {
    perror("Error saving currency state to file");
  }
  json_decref(ctx->json);
  free(ctx);
}

/**
 * @brief Allocates the context for a microsever of type currency.
 *
 * @param pointer to server structure.
 */
void _create_currency_context(server_t *serv) {
  currency_ctx_t *context = malloc(sizeof(currency_ctx_t));
  if (context == NULL) {
    perror("Failed micro-context allocation!");
    return;
  }
  serv->context = context;
  json_error_t json_load_error;
  json_t *currency_json = json_load_file(CURRENCY_JSON_FILE, 0, &json_load_error);
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
bool _get_city_weather(weather_ctx_t *context, const string_t *city, response_weather_t *resp) {
  json_t *weather_json = json_object_get(context->json, str_to_cstr(city));
  if (weather_json == NULL) {
    perror("Error trying to fetch weather info for city");
    return false;
  }
  // This doesn't lose memory because of borrowed references.
  // https://jansson.readthedocs.io/en/2.10/apiref.html#c.json_decref
  resp->humidity = json_integer_value(json_object_get(weather_json, "humidity"));
  resp->pressure = json_real_value(json_object_get(weather_json, "pressure"));
  resp->temperature = json_real_value(json_object_get(weather_json, "temperature"));

  return true;
}

/**
 * @brief Updates the weather context with new valid values in a request.
 *
 * @param weather context of server (w/ structures), city name and ptr to struct weather **request**.
 */
bool _set_city_weather(weather_ctx_t *context, const string_t *city, const request_t *r) {
  json_t *weather_json = json_object_get(context->json, str_to_cstr(city));
  if (weather_json == NULL) {
    perror("Error trying to fetch weather info for city");
    return false;
  }
  // We could define some macros to make this code less cluttered.
  if (VALID(r->u.post_weather.humidity))
    json_integer_set(json_object_get(weather_json, "humidity"), r->u.post_weather.humidity);
  if (VALID(r->u.post_weather.pressure))
    json_real_set(json_object_get(weather_json, "pressure"), r->u.post_weather.pressure);
  if (VALID(r->u.post_weather.temperature))
    json_real_set(json_object_get(weather_json, "temperature"), r->u.post_weather.temperature);

  return true;
}

/**
 * @brief Request callback of weather microservice.
 *
 * @param Pointers to response, request and server struct.
 */
static void _handle_weather(response_t *resp, const request_t *r, const server_t *serv) {
  printf("[MICROSERVICE] got request: ");
  request_print(r);

  weather_ctx_t *context = ( weather_ctx_t * )serv->context;
  if (r->type == request_post_weather) {
    resp->type = response_result;

    // Set weather.
    if (!_set_city_weather(context, &r->u.weather.city, r)) {
      str_init(&resp->u.result.message, "Failed");
    } else {
      str_init(&resp->u.result.message, "Success");
    }
  } else {
    // Get weather status.
    resp->type = response_weather;
    if (!_get_city_weather(context, &r->u.weather.city, &resp->u.weather)) {
      resp->type = response_result;
      str_init(&resp->u.result.message, "Not found");
    }
  }
}

/**
 * @brief Obtains the exchange value for a given currency.
 *
 * @param currency context of server (w/ structures) and currency name.
 */
bool _get_currency_exchange(currency_ctx_t *context, const string_t *currency, float_t *exchange) {
  json_t *value = json_object_get(context->json, str_to_cstr(currency));
  if (value == NULL) {
    return false;
  }

  *exchange = json_real_value(value);
  return true;
}

/**
 * @brief Updates the currency context with new valid values in a request.
 *
 * @param currency context of server (w/ structures), city name and ptr to struct currency **request**.
 */
bool _set_currency_exchange(currency_ctx_t *context, const string_t *currency, const request_t *r) {
  json_t *currency_json = json_object_get(context->json, str_to_cstr(currency));
  if (currency_json == NULL) {
    perror("Error trying to fetch currency info for coin");
    return false;
  }
  // As before, we could re-use some macros here.
  if (VALID(r->u.post_currency.value))
    json_real_set(currency_json, r->u.post_currency.value);

  return true;
}

/**
 * @brief Handle the currency micro service request.
 *
 * @param response and request structures, along with server structure.
 */
static void _handle_currency(response_t *resp, const request_t *r, const server_t *serv) {
  printf("[MICROSERVICE] got request: ");
  request_print(r);

  currency_ctx_t *context = ( currency_ctx_t * )serv->context;
  if (r->type == request_post_currency) {
    resp->type = response_result;

    printf("Updating currency value to %f\n", r->u.post_currency.value);
    if (!_set_currency_exchange(context, &r->u.currency.currency, r)) {
      str_init(&resp->u.result.message, "Failed");
    } else {
      str_init(&resp->u.result.message, "Success");
    }
  } else {
    if (_get_currency_exchange(context, &r->u.currency.currency, &resp->u.currency.quote)) {
      resp->type = response_currency;
    } else {
      resp->type = response_result;
      str_init(&resp->u.result.message, "Not found");
    }
  }
}

/**
 * @brief Handle the micro service request.
 *
 * @param response and request structures.
 */
static void _micro_handle_request(response_t *resp, const request_t *r, const server_t *serv) {

  // A valid question is: why define the server's type if we still
  // have to switch() to get the corresponding behavior?
  // Well, while it is true, it doesn't look like a true microservice
  // if a process can handle any request.
  // Furthermore, in the future we might add more context to the server struct
  // (pending discussion on PR #9)

  if (get_base_request(r->type) != serv->type && r->type != request_last) {
    perror("Sent request to wrong server!");
    return;
  }

  /* Prop: Use a hash in order to save all pairs (city, weather) or
   * (coin, value) */

  switch (get_base_request(serv->type)) {
    case request_weather:
      _handle_weather(resp, r, serv);
      break;
    case request_currency:
      _handle_currency(resp, r, serv);
      break;
    default:
      break;
  }
}
/**
 * @brief Maps requests types to one of request_weather, _currency or _last.
 *
 * @param request type to be mapped.
 */
inline request_type_t get_base_request(request_type_t type) {
  if (type == request_post_currency)
    return request_currency;

  if (type == request_post_weather)
    return request_weather;

  return type;
}
/**
 * @brief Launches and executes the main loop of the microservice.
 *
 * @param type of microservice and pointer to the exit flag that is
 * modified on signal (server/main.c:sigint_handler).
 */
int launch_microservice(request_type_t type, bool *exit_flag) {
  server_t microserver;

  microserver.type = type;
  int port = SELF_PORT + type + 1; // Add +1, since enums start at 0.

  if (!server_init(&microserver, port, _micro_handle_request)) {
    perror("Error al iniciar el microservicio");
    return 1;
  }

  if (type == request_weather) {
    _create_weather_context(&microserver);
  } else if (type == request_currency) {
    _create_currency_context(&microserver);
  }

  /* handles client (portal, middleware's) requests */
  while (!*exit_flag) {
    printf("waiting connection...\n");
    if (!server_handle_request(&microserver)) {
      break;
    }
  }

  if (type == request_weather) {
    // If flag is true, weather state is saved to file.
    _finish_weather_service(&microserver, *exit_flag);
  } else if (type == request_currency) {
    // Same, but with currencies.
    _finish_currency_service(&microserver, *exit_flag);
  }

  server_stop(&microserver);
  return 0;
}
