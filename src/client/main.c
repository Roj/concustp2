/* include area */
#include "client.h"
#include "str.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define SERVER_PORT 8002

#define SECRET_PASSWORD "concutp2"
#define INVALID_VALUE -999

/**
 * @brief Prints a field through STDOUT.
 *
 * @param field Field to print.
 * @param desc Field description.
 * @param cb_ctx Unused.
 * @return false on error, true on success.
 */
static bool _field_print(const void *field, const field_desc_t *desc, void *cb_ctx) {
  /* gets the required buffer size to serialize the field */
  size_t size = field_to_cstr(NULL, 0, field, desc->type);

  /* serializes the field */
  char field_cstr[size];
  if (field_to_cstr(field_cstr, size, field, desc->type) <= 0) {
    return false;
  }

  /* outputs the field name and value */
  printf("%s: %s\n", desc->name, field_cstr);
  return true;
}

/**
 * @brief Prints a response through STDOUT in pairs "field name": "field value".
 *
 * @param r Response to print.
 * @return false on error, true on success.
 */
static bool _print_response(const response_t *r) {
  /* gets the response's description */
  const message_desc_t *desc = &response_descs[r->type];

  /* iterates the response printing the fields */
  return message_iter_const(&r->u, desc, _field_print, NULL);
}

/**
 * @brief Prints the help callable with the "--help" flag
 */
static void _print_help( ) {
  printf("Usage: client REQ_TYPE REQ_TARGET [OPTIONS...]\n");
  printf("\nAvailable REQ_TYPEs: \n");
  printf("Get weather : %d\nGet currency : %d \n", request_weather + 1, request_currency + 1);
  printf("Post weather : %d\nPost currency : %d \n", request_post_weather + 1, request_post_currency + 1);
  printf("\nAvailable OPTIONS: \n");
  printf("--pass: Defines password to attemp a post as administrator\n");
  printf("--c : Defines currency value (fails if administrator cannot authenticate)\n");
  printf("--p : Defines presure value (fails if administrator cannot authenticate)\n");
  printf("--t : Defines temperature value (fails if administrator cannot authenticate)\n");
  printf("--h : Defines humidity value (fails if administrator cannot authenticate)\n");
  printf("\nSome usage examples:\n");
  printf("client %d \"buenos aires\" : Retrieves \"buenos aires\" city weather\n", request_weather + 1);
  printf("client %d \"Dolar\" : Retrieves \"Dolar\" currency value\n", request_currency + 1);
  printf("client %d \"Dolar\" --pass PASS --c 18.00 : Sets \"Dolar\" currency value to 18 (if PASS is "
         "correct)\n",
         request_post_currency + 1);
  printf("client %d \"buenos aires\" --p 1001.1 --t 26.2 --pass PASS : Sets \"buenos aires\" presure to "
         "1001.1 and temperature to 26.2 (if PASS is correct)\n",
         request_post_weather + 1);
}

/**
 * @brief Checks if the given password is a valid one.
 *
 * @param pass password to check
 * @return true if password is valid, false otherwise
 */
static bool _password_is_valid(const char *pass) {
  return (!strcmp(pass, SECRET_PASSWORD));
}

/**
 * @brief Prints a parsing error message.
 */
static void _print_error_parsing( ) {
  printf("Failed initializing request: Wrong arguments\n");
  printf("Try rerunning using the --help flag for help\n");
}

/**
 * @brief Fills the request parsing argc and argv. This function also
 * checks if a POST attempt from a self-claimed administrator can be
 * done (i.e. if the password given is correct).
 *
 * @param req request_t struct to fill
 * @param argc argc value from main
 * @param argv argv array from main
 * @return false on error, true on success
 */
static bool _make_request(request_t *req, int argc, const char *argv[]) {
  float value;
  char *endptr;
  switch (req->type) {
    // Client (GET) requests
    case request_currency:
      str_init(&req->u.currency.currency, argv[2]);
      return true;
    case request_weather:
      str_init(&req->u.weather.city, argv[2]);
      return true;
    // Admin (POST) requests
    case request_post_currency:
      if (argc != 7) {
        _print_error_parsing( );
        return false;
      }
      str_init(&req->u.post_currency.currency, argv[2]);
      for (int i = 3; i < argc - 1; i += 2) {
        if (!strcmp(argv[i], "--pass")) {
          if (!_password_is_valid(argv[i + 1])) {
            printf("Authentication error: Password invalid!\n");
            return false;
          }
        } else if (!strcmp(argv[i], "--c")) {
          value = ( float )strtod(argv[i + 1], &endptr);
          if (strlen(endptr) || (value <= 0)) {
            _print_error_parsing( );
            return false;
          }
          req->u.post_currency.value = value;
        } else {
          _print_error_parsing( );
          return false;
        }
      }
      return true;
    case request_post_weather:
      if ((argc < 5) || ((argc - 3) % 2)) {
        _print_error_parsing( );
        return false;
      }

      /* Note there's no need to update all weather fields in
       * just one request. Those fields that mustn't be updated
       * with this request are set to INVALID_VALUE. The weather
       * server should then check if any of the received values
       * is INVALID_VALUE and not update it.*/
      str_init(&req->u.post_weather.city, argv[2]);
      req->u.post_weather.humidity = INVALID_VALUE;
      req->u.post_weather.pressure = INVALID_VALUE;
      req->u.post_weather.temperature = INVALID_VALUE;
      for (int i = 3; i < argc - 1; i += 2) {
        if (!strcmp(argv[i], "--pass")) {
          if (!_password_is_valid(argv[i + 1])) {
            printf("Authentication error: Password invalid!\n");
            return false;
          }
        } else if (!strcmp(argv[i], "--p")) {
          value = ( float )strtod(argv[i + 1], &endptr);
          if (strlen(endptr) || (value <= 0)) {
            _print_error_parsing( );
            return false;
          }
          req->u.post_weather.pressure = value;
        } else if (!strcmp(argv[i], "--t")) {
          value = ( float )strtod(argv[i + 1], &endptr);
          if (strlen(endptr) || (value <= -273)) {
            _print_error_parsing( );
            return false;
          }
          req->u.post_weather.temperature = value;
        } else if (!strcmp(argv[i], "--h")) {
          value = ( float )strtod(argv[i + 1], &endptr);
          if (strlen(endptr) || (value <= 0)) {
            _print_error_parsing( );
            return false;
          }
          req->u.post_weather.humidity = value;
        } else {
          _print_error_parsing( );
          return false;
        }
      }
      return true;
    default:
      _print_error_parsing( );
      return false;
  }
}

/**
 * @brief Parses argc and argv in order to create a request to the server.
 *
 * @param req request_t struct to fill
 * @param argc argc value from main
 * @param argv argv array from main
 * @return false on error, true on success.
 */
static bool _parse_options(request_t *req, int argc, const char *argv[]) {
  char *endptr;
  int rt;
  if (argc == 2) {
    if (strcmp(argv[1], "--help"))
      _print_error_parsing( );
    else
      _print_help( );
    return false;
  } else if (argc > 2) {
    rt = ( int )strtol(argv[1], &endptr, 10);
    if (strlen(endptr) || (rt > request_last) || (rt <= 0)) {
      _print_error_parsing( );
      return false;
    }
    req->type = rt - 1;
    return _make_request(req, argc, argv);
  } else {
    _print_error_parsing( );
    return false;
  }
  return false;
}

int main(int argc, const char *argv[]) {
  /* initializes the request */
  request_t req = {0};

  if (!_parse_options(&req, argc, argv))
    return 1;

  /* sends the request and waits the response */
  response_t resp = {0};
  if (!client_send(&resp, SERVER_PORT, &req)) {
    perror("Failed sending the request");
    return 1;
  }

  /* prints the response */
  if (!_print_response(&resp)) {
    perror("Failed printing the response");
    return 1;
  }

  /* success */
  return 0;
}
