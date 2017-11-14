/* include area */
#include "requests.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief Stringifies a given value in compile time.
 */
#define _STR(value) #value
#define STR(value) _STR(value)

/**
 * @brief Serializes a field size and writes it through the output callback.
 *
 * @param field_size Value to be serialized.
 * @param out Callback where the output is written to.
 * @param out_ctx Pointer passed to the out callback.
 * @return false on error, true on success.
 */
static bool _field_size_serialize(size_t field_size, write_cb_t out, void *out_ctx) {
  char buffer[MAX_SIZE_IN_BYTES + 1];
  if (snprintf(buffer, sizeof(buffer), "%0" STR(MAX_SIZE_IN_BYTES) "zu", field_size) >= sizeof(buffer))
    return false;

  return out(buffer, MAX_SIZE_IN_BYTES, out_ctx);
}

static bool _parse_char(size_t *out, char c) {
  /* checks if it's a numeric value */
  if (c < '0' || c > '9')
    return false;

  /* converts to size_t */
  *out = c - '0';
  return true;
}

static bool _field_size_deserialize(size_t *field_size, read_cb_t in, void *in_ctx) {
  char buffer[MAX_SIZE_IN_BYTES];
  if (!in(buffer, sizeof(buffer), in_ctx) >= sizeof(buffer))
    return false;

  /* parses the field size */
  *field_size = 0;
  for (size_t i = 0; i < MAX_SIZE_IN_BYTES; i++) {
    *field_size *= 10;

    size_t digit;
    if (!_parse_char(&digit, buffer[i]))
      return false;

    *field_size += digit;
  }

  return true;
}

/**
 * @brief Writes a serialized field through the out callback.
 *
 * @param field Field to serialize and output.
 * @param type Field type.
 * @param out Callback that outputs the serialized content.
 * @param out_ctx Pointer passed to out.
 * @return false on error, true on success.
 */
static bool _write_field(const void *field, field_type_t type, write_cb_t out, void *out_ctx) {
  /* gets the number of bytes required to serialize the field */
  size_t buffer_size = field_to_cstr(NULL, 0, field, type);
  if (buffer_size == 0)
    return false;

  /* serializes the field */
  char buffer[buffer_size];
  if (field_to_cstr(buffer, buffer_size, field, type) == 0)
    return false;

  /* outputs the buffer size and it's content */
  if (!_field_size_serialize(buffer_size - 1, out, out_ctx))
    return false;

  /* writes the field excluding the terminating null character */
  return out(buffer, buffer_size - 1, out_ctx);
}

static bool _read_field(void *field, field_type_t type, read_cb_t in, void *in_ctx) {
  /* gets the number of bytes used to serialize the field */
  size_t field_size;
  if (!_field_size_deserialize(&field_size, in, in_ctx))
    return false;

  char buffer[field_size + 1];
  if (!in(buffer, field_size, in_ctx))
    return false;

  /* appends the terminating null character */
  buffer[field_size] = '\0';

  /* deserializes */
  return field_from_cstr(field, type, buffer);
}

/**
 * @brief Serializes a currency request.
 *
 * @param r Currency request to serialize.
 * @param out Callback to write the output.
 * @param out_ctx Pointer passed to out.
 * @return false on error, true on success.
 */
static bool _serialize_currency_req(const currency_req_t *r, write_cb_t out, void *out_ctx) {
  return _write_field(&r->currency, field_type_string, out, out_ctx);
}

static bool _deserialize_currency_req(currency_req_t *r, read_cb_t in, void *in_ctx) {
  return _read_field(&r->currency, field_type_string, in, in_ctx);
}

/**
 * @brief Serializes a weather request.
 *
 * @param r Currency request to serialize.
 * @param out Callback to write the output.
 * @param out_ctx Pointer passed to out.
 * @return false on error, true on success.
 */
static bool _serialize_weather_req(const weather_req_t *r, write_cb_t out, void *out_ctx) {
  return _write_field(&r->city, field_type_string, out, out_ctx);
}

static bool _deserialize_weather_req(weather_req_t *r, read_cb_t in, void *in_ctx) {
  return _read_field(&r->city, field_type_string, in, in_ctx);
}

static bool _serialize_currency_resp(const currency_resp_t *r, write_cb_t out, void *out_ctx) {
  return _write_field(&r->quote, field_type_float, out, out_ctx);
}

static bool _deserialize_currency_resp(currency_resp_t *r, read_cb_t in, void *in_ctx) {
  return _read_field(&r->quote, field_type_float, in, in_ctx);
}

static bool _serialize_weather_resp(const weather_resp_t *r, write_cb_t out, void *out_ctx) {
  if (!_write_field(&r->humidity, field_type_integer, out, out_ctx))
    return false;
  if (!_write_field(&r->pressure, field_type_float, out, out_ctx))
    return false;
  if (!_write_field(&r->temperature, field_type_float, out, out_ctx))
    return false;
  return true;
}

static bool _deserialize_weather_resp(weather_resp_t *r, read_cb_t in, void *in_ctx) {
  if (!_read_field(&r->humidity, field_type_integer, in, in_ctx))
    return false;
  if (!_read_field(&r->pressure, field_type_float, in, in_ctx))
    return false;
  if (!_read_field(&r->temperature, field_type_float, in, in_ctx))
    return false;
  return true;
}

/**
 * @brief Serializes a request sending the output through the "out" callback.
 *
 * @param r Request to serialize.
 * @param out Callback that outputs the serialized data.
 * @param out_ctx Pointer passed to out.
 * @return false on error, true on success.
 */
bool request_serialize(const request_t *r, write_cb_t out, void *out_ctx) {
  /* writes the type as the first byte */
  char type = r->type;
  if (!out(&type, 1, out_ctx))
    return false;

  switch (r->type) {
    case req_currency:
      return _serialize_currency_req(&r->u.currency, out, out_ctx);
    case req_weather:
      return _serialize_weather_req(&r->u.weather, out, out_ctx);
  }

  /* invalid request type */
  return false;
}

/**
 * @brief Parses a request reading the content from the "in" callback.
 *
 * @param r Parsed request (output).
 * @param in Callback that gives the data to parse.
 * @param in_ctx Pointer passed to out.
 * @return false on error, true on success.
 */
bool request_deserialize(request_t *r, read_cb_t in, void *in_ctx) {
  /* the first byte indicates the type */
  char type;
  if (!in(&type, sizeof(type), in_ctx))
    return false;

  /* sets the type */
  r->type = type;

  switch (r->type) {
    case req_currency:
      return _deserialize_currency_req(&r->u.currency, in, in_ctx);
    case req_weather:
      return _deserialize_weather_req(&r->u.weather, in, in_ctx);
  }

  /* invalid request type */
  return false;
}

bool response_serialize(const response_t *r, write_cb_t out, void *out_ctx) {
  /* writes the type as the first byte */
  char type = r->type;
  if (!out(&type, 1, out_ctx))
    return false;

  switch (r->type) {
    case resp_currency:
      return _serialize_currency_resp(&r->u.currency, out, out_ctx);
    case resp_weather:
      return _serialize_weather_resp(&r->u.weather, out, out_ctx);
  }

  /* invalid response type */
  return false;
}

bool response_deserialize(response_t *r, read_cb_t in, void *in_ctx) {
  /* the first byte indicates the type */
  char type;
  if (!in(&type, sizeof(type), in_ctx))
    return false;

  /* sets the type */
  r->type = type;

  switch (r->type) {
    case resp_currency:
      return _deserialize_currency_resp(&r->u.currency, in, in_ctx);
    case resp_weather:
      return _deserialize_weather_resp(&r->u.weather, in, in_ctx);
  }

  /* invalid response type */
  return false;
}
