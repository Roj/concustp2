/* include area */
#include "requests.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief Stringifies a given value in compile time.
 */
#define _STR(value) #value
#define STR(value) _STR(value)

/** Serialization context. */
typedef struct {
  /** The output callback. */
  write_cb_t out;
  /** The output callback context. */
  void *out_ctx;
} serialization_ctx_t;

/** Deerialization context. */
typedef struct {
  /** The input callback. */
  read_cb_t in;
  /** The input callback context. */
  void *in_ctx;
} deserialization_ctx_t;

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
 * @param desc Field description.
 * @param cb_ctx Callback context.
 * @return false on error, true on success.
 */
static bool _field_serialize(const void *field, const field_desc_t *desc, void *cb_ctx) {
  serialization_ctx_t *ctx = cb_ctx;

  /* gets the number of bytes required to serialize the field */
  size_t buffer_size = field_to_cstr(NULL, 0, field, desc->type);
  if (buffer_size == 0)
    return false;

  /* serializes the field */
  char buffer[buffer_size];
  if (field_to_cstr(buffer, buffer_size, field, desc->type) == 0)
    return false;

  /* outputs the buffer size and it's content */
  if (!_field_size_serialize(buffer_size - 1, ctx->out, ctx->out_ctx))
    return false;

  /* writes the field excluding the terminating null character */
  return ctx->out(buffer, buffer_size - 1, ctx->out_ctx);
}

/**
 * @brief Reads a serialized field and deserializes it.
 *
 * @param field Field to deserialize.
 * @param desc Field description.
 * @param cb_ctx Callback context.
 * @return false on error, true on success.
 */
static bool _field_deserialize(void *field, const field_desc_t *desc, void *cb_ctx) {
  deserialization_ctx_t *ctx = cb_ctx;

  /* gets the number of bytes used to serialize the field */
  size_t field_size;
  if (!_field_size_deserialize(&field_size, ctx->in, ctx->in_ctx))
    return false;

  char buffer[field_size + 1];
  if (!ctx->in(buffer, field_size, ctx->in_ctx))
    return false;

  /* appends the terminating null character */
  buffer[field_size] = '\0';

  /* deserializes */
  return field_from_cstr(field, desc->type, buffer);
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
  if (type >= request_last) {
    return false;
  }

  if (!out(&type, 1, out_ctx))
    return false;

  serialization_ctx_t iter_ctx = {.out = out, .out_ctx = out_ctx};
  const message_desc_t *desc = &request_descs[r->type];
  return message_iter_const(&r->u, desc, _field_serialize, &iter_ctx);
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
  if (type >= request_last) {
    return false;
  }

  deserialization_ctx_t iter_ctx = {.in = in, .in_ctx = in_ctx};
  const message_desc_t *desc = &request_descs[r->type];
  return message_iter(&r->u, desc, _field_deserialize, &iter_ctx);
}

/**
 * @brief Serializes a response, writing the serialized content through
 * the given output callback.
 *
 * @param r Response to serialize (properly initialized).
 * @param out Output callback.
 * @param out_ctx Output callback context.
 * @return false on error, true on success.
 */
bool response_serialize(const response_t *r, write_cb_t out, void *out_ctx) {
  /* writes the type as the first byte */
  char type = r->type;
  if (type >= response_last) {
    return false;
  }

  if (!out(&type, 1, out_ctx))
    return false;

  serialization_ctx_t iter_ctx = {.out = out, .out_ctx = out_ctx};
  const message_desc_t *desc = &response_descs[r->type];
  return message_iter_const(&r->u, desc, _field_serialize, &iter_ctx);
}

/**
 * @brief Parses a response reading the content from the "in" callback.
 *
 * @param r Parsed response (output).
 * @param in Callback that gives the data to parse.
 * @param in_ctx Pointer passed to out.
 * @return false on error, true on success.
 */
bool response_deserialize(response_t *r, read_cb_t in, void *in_ctx) {
  /* the first byte indicates the type */
  char type;
  if (!in(&type, sizeof(type), in_ctx))
    return false;

  /* sets the type */
  r->type = type;
  if (type >= response_last) {
    return false;
  }

  deserialization_ctx_t iter_ctx = {.in = in, .in_ctx = in_ctx};
  const message_desc_t *desc = &response_descs[r->type];
  return message_iter(&r->u, desc, _field_deserialize, &iter_ctx);
}
