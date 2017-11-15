#ifndef REQUESTS_H
#define REQUESTS_H

/* include area */
#include "message.h"
#include "types.h"

/**
 * @brief Request/response field definitions
 */
#define WEATHER_REQ_FIELDS(request_name) FIELD(request_name, city, string)

#define CURRENCY_REQ_FIELDS(request_name) FIELD(request_name, currency, string)

#define WEATHER_RESP_FIELDS(response_name)                                                                   \
  FIELD(response_name, humidity, integer)                                                                    \
  FIELD(response_name, pressure, float)                                                                      \
  FIELD(response_name, temperature, float)

#define CURRENCY_RESP_FIELDS(response_name) FIELD(response_name, quote, float)

/**
 * @brief List of requests and responses
 */
#define REQUESTS( )                                                                                          \
  ENTRY(weather, WEATHER_REQ_FIELDS)                                                                         \
  ENTRY(currency, CURRENCY_REQ_FIELDS)

#define RESPONSES( )                                                                                         \
  ENTRY(weather, WEATHER_RESP_FIELDS)                                                                        \
  ENTRY(currency, CURRENCY_RESP_FIELDS)

/*------------------------------------------------------------------------------
   Black magic
------------------------------------------------------------------------------*/

/**
 * @brief Generates the request types.
 */
#define MESSAGE_NAME request
#define MESSAGES REQUESTS( )

#include "message_decl.h"

#undef MESSAGES
#undef MESSAGE_NAME

/**
 * @brief Generates the response types.
 */
#define MESSAGE_NAME response
#define MESSAGES RESPONSES( )

#include "message_decl.h"

#undef MESSAGES
#undef MESSAGE_NAME

/*--------------------------------------------------------------------------
   Prototypes
--------------------------------------------------------------------------*/

/** IO prototypes */
bool request_serialize(const request_t *r, write_cb_t out, void *out_ctx);
bool request_deserialize(request_t *r, read_cb_t in, void *in_ctx);

bool response_serialize(const response_t *r, write_cb_t out, void *out_ctx);
bool response_deserialize(response_t *r, read_cb_t in, void *in_ctx);

#endif
