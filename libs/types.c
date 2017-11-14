/* include area */
#include "types.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Serializes a generic field into a C string.
 *
 * @param s Output string with the serialized field.
 * @param len Max length of the output buffer.
 *            If len is 0, then the number of bytes required to serialize the field are returned.
 * @param field Field to serialize.
 * @param type Type of the field to serialize.
 * @return The number of bytes in the output buffer or 0 on error.
 */
size_t field_to_cstr(char *s, size_t len, const void *field, field_type_t type) {
  switch (type) {
    case field_type_float:
      return float_to_cstr(s, len, field);

    case field_type_integer:
      return integer_to_cstr(s, len, field);

    case field_type_string:
      return string_to_cstr(s, len, field);
  }

  /* invalid field type */
  return 0;
}

/**
 * @brief Parses a generic field from a C string.
 *
 * @param field Pointer where the parsed field contents are stored.
 * @param type Type of the field to deserialize.
 * @param s C string with the serialized field.
 * @return false on error, true on success.
 */
bool field_from_cstr(void *field, field_type_t type, const char *s) {
  switch (type) {
    case field_type_float:
      return float_from_cstr(field, s);

    case field_type_integer:
      return integer_from_cstr(field, s);

    case field_type_string:
      return string_from_cstr(field, s);
  }

  /* invalid field type */
  return 0;
}

/**
 * @brief Parses an integer_t from a C string.
 *
 * @param i Output integer.
 * @param s Input string.
 * @return false if the string didn't contain a valid integer, true on success.
 */
bool integer_from_cstr(integer_t *i, const char *s) {
  return (sscanf(s, "%" SCNi32, i) == 1);
}

/**
 * @brief Serializes an integer into a C string.
 *
 * @param s Output string where the integer is serialized.
 * @param len Max length of the output buffer.
 * @param i Integer to serialize.
 * @return false if the string was not large enough to contain the serialized value, true on success.
 */
size_t integer_to_cstr(char *s, size_t len, const integer_t *i) {
  int bytes = snprintf(s, len, "%" PRIi32, *i);
  if (bytes < 0)
    return 0;

  if (len == 0)
    return bytes + 1;

  /* checks if the serialized value fitted into the output buffer */
  return (bytes < len);
}

/**
 * @brief Parses a float_t from a C string.
 *
 * @param f Output float.
 * @param s Input string.
 * @return false if the string didn't contain a valid float, true on success.
 */
bool float_from_cstr(float_t *f, const char *s) {
  return (sscanf(s, "%f", f) == 1);
}

/**
 * @brief Serializes a float into a C string.
 *
 * @param s Output string where the float is serialized.
 * @param len Max length of the output buffer.
 * @param f float to serialize.
 * @return false if the string was not large enough to contain the serialized value, true on success.
 */
size_t float_to_cstr(char *s, size_t len, const float_t *f) {
  int bytes = snprintf(s, len, "%.4f", *f);
  if (bytes < 0)
    return 0;

  if (len == 0)
    return bytes + 1;

  /* checks if the serialized value fitted into the output buffer */
  return (bytes < len);
}

/**
 * @brief Parses a string_t from a C string.
 *
 * @param st Output string_t.
 * @param s Input string.
 * @return false if the string didn't contain a valid string_t, true on success.
 */
bool string_from_cstr(string_t *st, const char *s) {
  size_t len = strlen(s);
  if (len > sizeof(st->buffer))
    return false;

  memcpy(st->buffer, s, len);
  st->length = len;
  return true;
}

/**
 * @brief Serializes a string_t into a C string.
 *
 * @param s Output string where the float is serialized.
 * @param len Max length of the output buffer.
 * @param st string to serialize.
 * @return false if the string was not large enough to contain the serialized value, true on success.
 */
size_t string_to_cstr(char *s, size_t len, const string_t *st) {
  if (len == 0)
    return st->length + 1;

  if (len <= st->length)
    return 0;

  /* copies the content */
  memcpy(s, st->buffer, st->length);
  s[st->length] = '\0';
  return st->length + 1;
}
