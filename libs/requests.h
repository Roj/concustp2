#ifndef REQUESTS_H
#define REQUESTS_H

/* include area */
#include "message.h"
#include "types.h"

/**
 * @brief List of requests and responses
 *
 * Once defined in REQUESTS or RESPONSES, the struct will be available
 * in request_t.u and response_t.u (and the corresponding request_type_t
 * enum is also generated).
 *
 * Also, the serialization/deserialization is handled automatically (no
 * need to define a (de)serialization function for every request/response).
 *
 * See message.h for more information.
 */
// clang-format off

#define REQUESTS( )\
  ENTRY(weather,                               \
    FIELD(weather, city, string))              \
  ENTRY(currency,                              \
    FIELD(currency, currency, string))         \
  ENTRY(post_weather,                          \
    FIELD(weather, city, string)               \
    FIELD(post_weather, humidity, float)       \
    FIELD(post_weather, pressure, float)       \
    FIELD(post_weather, temperature, float))   \
  ENTRY(post_currency,                         \
    FIELD(post_currency, currency, string)     \
    FIELD(post_currency, value, float))        \


#define RESPONSES( )                           \
  ENTRY(weather,                               \
    FIELD(weather, humidity, float)            \
    FIELD(weather, pressure, float)            \
    FIELD(weather, temperature, float))        \
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

/** Callback where a message is read from */
typedef size_t (*read_cb_t)(void *output, size_t bytes, void *cb_ctx);

/** Callback where a message is written to */
typedef bool (*write_cb_t)(const void *data, size_t bytes, void *cb_ctx);

/** IO prototypes */
bool request_serialize(const request_t *r, write_cb_t out, void *out_ctx);
bool request_deserialize(request_t *r, read_cb_t in, void *in_ctx);
void request_print(const request_t *r);

bool response_serialize(const response_t *r, write_cb_t out, void *out_ctx);
bool response_deserialize(response_t *r, read_cb_t in, void *in_ctx);
void response_print(const response_t *r);

#endif
