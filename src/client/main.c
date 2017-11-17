/* include area */
#include "client.h"
#include "str.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT 8002

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
static void _print_help() {
	printf("Usage: client [REQ_TYPE] [REQ_TARGET]\n");
	printf("Available REQ_TYPEs: \n");
	printf("Weather: %d\nCurrency: %d \n", request_weather + 1, request_currency + 1);
}

/**
 * @brief Prints a parsing error message.
 */
static void _print_error_parsing() {
  printf("Failed initializing request: Wrong arguments\n");
  printf("Try rerunning using the --help flag for help\n");	
}

/**
 * @brief Parses argc and argv in order to create a request to the server.
 *
 * @param req request_t struct to fill
 * @param argc argc value from main
 * @param argv argv array from main
 * @return false on error, true on success.
 */
static bool _parse_request(request_t *req, int argc, const char *argv[]) {
  /* Prop: For easily implementing the "admin", we could just add one
   * extra argument in argv. Then, both admin and client would end up
   * using this same main file, and this parsing function would be the
   * one creating different type of requests for each of them.
   * For instance, one easy workaround would be reading argv[0]: if it's
   * "./client" or "./admin" behaviour of this function would change.
   * One more serious approach would include adding at the end of the
   * request an extra field like a "secret key". If that key matches
   * some specific value, that client can act as admin of the server.*/
  char *endptr;
  int rt;
  switch (argc) {
    case 2:
      if(strcmp(argv[1], "--help")) 
        _print_error_parsing();
	  else
        _print_help();
      return false;
    case 3:
      rt = (int) strtol(argv[1], &endptr, 10);
      if(strlen(endptr) || (rt > request_last) || (rt <= 0)) {
	    _print_error_parsing();
	    return false;
	  }
	  req->type = rt - 1;
	  str_init(&req->u.currency.currency, argv[2]);
	  return true;
    default:
      _print_error_parsing();
      return false;
  }
  return false;
}

int main(int argc, const char *argv[]) {
  /* initializes the request */
  request_t req = {0};
  
  req.type = request_currency;
  str_init(&req.u.currency.currency, "pesos");

  if(!_parse_request(&req, argc, argv))
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
