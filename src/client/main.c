/* include area */
#include "client.h"
#include "str.h"
#include <stdio.h>
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

int main(int argc, const char *argv[]) {
  /* initializes the request */
  request_t req = {0};
  req.type = request_currency;
  str_init(&req.u.currency.currency, "pesos");

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
