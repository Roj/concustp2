#ifndef TYPES_H
#define TYPES_H

/* include area */
#include "str.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/** Fields supported by the protocol. */
typedef enum {
  field_type_integer,
  field_type_float,
  field_type_string,
} field_type_t;

/** Type definitions */
typedef int32_t integer_t;
typedef float float_t;

/* deserialization */
bool field_from_cstr(void *field, field_type_t type, const char *s);
bool integer_from_cstr(integer_t *i, const char *s);
bool float_from_cstr(float_t *f, const char *s);
bool string_from_cstr(string_t *st, const char *s);

/* serialization */
size_t field_to_cstr(char *s, size_t len, const void *field, field_type_t type);
size_t integer_to_cstr(char *s, size_t len, const integer_t *i);
size_t float_to_cstr(char *s, size_t len, const float_t *f);
size_t string_to_cstr(char *s, size_t len, const string_t *st);

#endif
