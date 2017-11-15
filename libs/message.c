/* include area */
#include "message.h"
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
 * @brief Serializes a message.
 *
 * @param message Message struct (the union).
 * @param desc The corresponding message description.
 * @param out Write callback.
 * @param out_ctx Write callback context.
 * @return false on error, true on success.
 */
bool message_serialize(const void *message, const message_desc_t *desc, write_cb_t out, void *out_ctx) {
  /* iterates through the message fields serializing them */
  for (size_t i = 0; i < desc->num_fields; i++) {
    const void *field = (( uint8_t * )message) + desc->fields[i].offset;
    if (!_write_field(field, desc->fields[i].type, out, out_ctx)) {
      return false;
    }
  }

  /* success */
  return true;
}

/**
 * @brief Deserializes a message.
 *
 * @param message Message struct (the union, output).
 * @param desc The corresponding message description.
 * @param in Read callback.
 * @param in_ctx Read callback context.
 * @return false on error, true on success.
 */
bool message_deserialize(void *message, const message_desc_t *desc, read_cb_t in, void *in_ctx) {
  /* iterates through the message fields serializing them */
  for (size_t i = 0; i < desc->num_fields; i++) {
    void *field = (( uint8_t * )message) + desc->fields[i].offset;
    if (!_read_field(field, desc->fields[i].type, in, in_ctx)) {
      return false;
    }
  }

  /* success */
  return true;
}
