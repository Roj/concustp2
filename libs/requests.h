#ifndef REQUESTS_H
#define REQUESTS_H

/* include area */
#include "types.h"
#include <stddef.h>

/** The number of bytes used to serialize the field size */
#define MAX_SIZE_IN_BYTES 4

/** Returns the number of elements in an *array* (doesn't work on pointers!!!) */
#define ASIZE(array) (sizeof(array) / sizeof(array[0]))

/**
 * @brief Request field definitions
 */
#define WEATHER(request_name) Y(request_name, city, string)

#define CURRENCY(request_name) Y(request_name, currency, string)

/**
 * @brief List of requests
 */
#define REQUESTS( )                                                                                          \
  X(weather, WEATHER)                                                                                        \
  X(currency, CURRENCY)

/*------------------------------------------------------------------------------
   Dark magic
------------------------------------------------------------------------------*/

/** Request types
 *  Defines an enum for each request. The enums are named "req_<req_name>""
 */
#define Y(...)
#define X(name, _) req_##name,

typedef enum { REQUESTS( ) req_last } req_type_t;

#undef X
#undef Y

/** Response types */
typedef enum {
  resp_weather,
  resp_currency,
} resp_type_t;

/** Field description. */
struct field_desc {
  /** Field type */
  field_type_t type;
  /** offset (in bytes) of the field from the beggining of the struct */
  size_t offset;
};

/** Message description. */
struct message_desc {
  /** Array of field descriptions (one for each field in the struct). */
  const struct field_desc *fields;
  /** Number of fields (i.e. elements in the array "fields"). */
  size_t num_fields;
};

/**
 * @brief Request structs definitions.
 * Defines a struct containing a field of the type specified in REQUESTS
 */
#define Y(request_name, field_name, field_type) field_type##_t field_name;
#define X(name, fields)                                                                                      \
  typedef struct {                                                                                           \
    fields(name##_t)                                                                                         \
  } name##_req_t;

REQUESTS( )

#undef X
#undef Y

/**
 * @brief Array of field_desc structs { type, offset } for each request.
 */
#define Y(request_name, field_name, field_type)                                                              \
  {.type = field_type_##field_type, .offset = offsetof(request_name, field_name)},
#define X(name, fields) static const struct field_desc name##fields_list[] = {fields(name##_req_t)};

REQUESTS( )

#undef X
#undef Y

/**
 * @brief Array of message_desc used to iterate the structs (indexed per request type).
 */
#define Y(request_name, field_name, field_type)
#define X(name, _) [req_##name] = {.num_fields = ASIZE(name##fields_list), .fields = name##fields_list},

static const struct message_desc request_descs[] = {REQUESTS( )};

#undef X
#undef Y

/** Response to a weather request */
typedef struct {
  float_t temperature;
  float_t pressure;
  integer_t humidity;
} weather_resp_t;

/** Currency quote response */
typedef struct {
  float_t quote;
} currency_resp_t;

/** Generic request type */
#define Y(...)
#define X(name, _) name##_req_t name;

typedef struct {
  req_type_t type;
  union {
    REQUESTS( )
  } u;
} request_t;

#undef Y
#undef X

/** Generic response type */
typedef struct {
  resp_type_t type;
  union {
    weather_resp_t weather;
    currency_resp_t currency;
  } u;
} response_t;

/** Callback where a response/request is read from */
typedef bool (*read_cb_t)(void *output, size_t bytes, void *cb_ctx);

/** Callback where a response/request is written to */
typedef bool (*write_cb_t)(const void *data, size_t bytes, void *cb_ctx);

/** IO prototypes */
bool request_serialize(const request_t *r, write_cb_t out, void *out_ctx);
bool request_deserialize(request_t *r, read_cb_t in, void *in_ctx);

bool response_serialize(const response_t *r, write_cb_t out, void *out_ctx);
bool response_deserialize(response_t *r, read_cb_t in, void *in_ctx);

#endif
