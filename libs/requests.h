#ifndef REQUESTS_H
#define REQUESTS_H

/* include area */
#include "message.h"
#include "types.h"

/**
 * @brief List of requests and responses
 */
// clang-format off

#define REQUESTS( )\
  ENTRY(weather,                               \
    FIELD(weather, city, string))         \
  ENTRY(currency,                              \
    FIELD(currency, currency, string))

#define RESPONSES( )                           \
  ENTRY(weather,                               \
    FIELD(weather, humidity, integer)    \
    FIELD(weather, pressure, float)      \
    FIELD(weather, temperature, float))  \
  ENTRY(currency,                              \
    FIELD(currency, quote, float))

// clang-format on

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
