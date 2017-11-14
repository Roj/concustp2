#ifndef REQUESTS_H
#define REQUESTS_H

/* include area */
#include "types.h"

/** The number of bytes used to serialize the field size */
#define MAX_SIZE_IN_BYTES 4

/** Request types */
typedef enum {
  req_weather,
  req_currency,
} req_type_t;

/** Response types */
typedef enum {
  resp_weather,
  resp_currency,
} resp_type_t;

/** Weather request */
typedef struct { string_t city; } weather_req_t;

/** Response to a weather request */
typedef struct {
  float_t temperature;
  float_t pressure;
  integer_t humidity;
} weather_resp_t;

/** Currency quote request */
typedef struct { string_t currency; } currency_req_t;

/** Currency quote response */
typedef struct { float_t quote; } currency_resp_t;

/** Generic request type */
typedef struct {
  req_type_t type;
  union {
    weather_req_t weather;
    currency_req_t currency;
  } u;
} request_t;

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
