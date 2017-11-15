/* include area */
#include "requests.h"

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

  const message_desc_t *desc = &request_descs[r->type];
  return message_serialize(&r->u, desc, out, out_ctx);
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

  const message_desc_t *desc = &request_descs[r->type];
  return message_deserialize(&r->u, desc, in, in_ctx);
}

bool response_serialize(const response_t *r, write_cb_t out, void *out_ctx) {
  /* writes the type as the first byte */
  char type = r->type;
  if (type >= response_last) {
    return false;
  }

  if (!out(&type, 1, out_ctx))
    return false;

  const message_desc_t *desc = &response_descs[r->type];
  return message_serialize(&r->u, desc, out, out_ctx);
}

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

  const message_desc_t *desc = &response_descs[r->type];
  return message_deserialize(&r->u, desc, in, in_ctx);
}
