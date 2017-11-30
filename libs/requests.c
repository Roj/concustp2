/* include area */
#include "buffer.h"
#include "requests.h"
#include <jansson.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Stringifies a given value in compile time.
 */
#define _STR(value) #value
#define STR(value) _STR(value)

#define MSG_TYPE_KEY "@type"

#define MAX_SERIALIZED_SIZE_LENGTH 128

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

static int _json_dump_cb(const char *buffer, size_t size, void *data) {
  buffer_t *b = data;
  return buffer_append(b, buffer, size) ? 0 : -1;
}

/**
 * @brief Output callback for request/response serialization that prints to STDOUT.
 * 
 * @param data Buffer to be printed.
 * @param size Buffer length.
 * @param ignored Unused.
 * @return Always true.
 */
static bool _stdout_print_cb(const void *data, size_t size, void *ignored) {
  printf("%*s", ( int )size, ( const char * )data);
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
  json_t *json = cb_ctx;

  json_t *json_field = NULL;
  switch (desc->type) {
    case field_type_integer:
      json_field = json_integer(*( integer_t * )field);
      break;
    case field_type_float:
      json_field = json_real(*( float_t * )field);
      break;
    case field_type_string:
      json_field = json_string(str_to_cstr(field));
      break;
  }

  if (json_field == NULL) {
    return false;
  }

  /* sets the JSON encoded fields into the output object */
  if (json_object_set_new_nocheck(json, desc->name, json_field) != 0) {
    json_decref(json_field);
    return false;
  }

  /* success */
  return true;
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
  json_t *json = cb_ctx;

  /* gets the corresponding JSON key */
  json_t *json_field = json_object_get(json, desc->name);
  if (json_field == NULL) {
    return false;
  }

  switch (desc->type) {
    case field_type_integer:
      if (json_typeof(json_field) != JSON_INTEGER) {
        return false;
      }
      *( integer_t * )field = json_integer_value(json_field);
      return true;
    case field_type_float:
      if (json_typeof(json_field) != JSON_REAL) {
        return false;
      }
      *( float_t * )field = json_real_value(json_field);
      return true;
    case field_type_string:
      if (json_typeof(json_field) != JSON_STRING) {
        return false;
      }
      return str_init(field, json_string_value(json_field));
  }

  /* unreachable */
  return false;
}

/**
 * @brief Converts a message into a JSON object.
 *
 * @param msg Message struct to convert (the union).
 * @param desc Message description.
 * @return JSON object on success, NULL on error.
 */
static json_t *_message_to_json(const void *msg, const message_desc_t *desc) {
  /* creates the JSON object that will hold the message */
  json_t *json = json_object( );
  if (json_object_set_nocheck(json, MSG_TYPE_KEY, json_string_nocheck(desc->name)) != 0) {
    json_decref(json);
    return NULL;
  }

  if (!message_iter_const(msg, desc, _field_serialize, json)) {
    json_decref(json);
    return NULL;
  }

  /* returns the JSON object */
  return json;
}

/**
 * @brief Loads a message from a JSON object.
 *
 * @param msg Message to load (the union).
 * @param desc Message description.
 * @param json JSON object where the message is loaded from.
 * @return false on error.
 */
static bool _message_from_json(void *msg, const message_desc_t *desc, json_t *json) {
  /* deserializes from the JSON object */
  return message_iter(msg, desc, _field_deserialize, json);
}

/**
 * @brief Serializes a message and sends it through an output callback.
 *
 * @param msg Message to serialize (the union).
 * @param desc Message description.
 * @param out Output callback.
 * @param out_ctx Output callback context.
 * @return false on error.
 */
static bool _message_serialize(const void *msg, const message_desc_t *desc, write_cb_t out, void *out_ctx) {
  /* creates the JSON object that will hold the message */
  json_t *json = _message_to_json(msg, desc);
  if (json == NULL) {
    return false;
  }

  /* dumps to string through the output callback */
  buffer_t b;
  buffer_init(&b);

  bool success = json_dump_callback(json, _json_dump_cb, &b, JSON_DISABLE_EOF_CHECK) == 0;
  json_decref(json);

  if(!success) {
    return false;
  }

  /* sends the JSON */
  return out(buffer_get_data(&b), buffer_get_len(&b), out_ctx);
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

  /* serialization */
  const message_desc_t *desc = &request_descs[r->type];
  return _message_serialize(&r->u, desc, out, out_ctx);
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
  /* deserializes the JSON object from the input callback */
  json_t *json = json_load_callback(in, in_ctx, JSON_DISABLE_EOF_CHECK, NULL);
  if (json == NULL) {
    return false;
  }

  /* gets the request type */
  json_t *type = json_object_get(json, MSG_TYPE_KEY);
  if (type == NULL || json_typeof(type) != JSON_STRING) {
    json_decref(json);
    return false;
  }

  /* gets the coresponding description */
  r->type = request_last;
  const char *type_cstr = json_string_value(type);
  for (request_type_t t = 0; t < request_last; t++) {
    if (strcmp(type_cstr, request_descs[t].name) == 0) {
      r->type = t;
      break;
    }
  }

  if (r->type == request_last) {
    json_decref(json);
    return false;
  }

  /* deserializes from the JSON object */
  const message_desc_t *desc = &request_descs[r->type];
  bool success = _message_from_json(&r->u, desc, json);

  json_decref(json);
  return success;
}

/**
 * @brief Prints a request through STDOUT.
 *
 * @param r Request to print.
 */
void request_print(const request_t *r) {
  /* serialization */
  const message_desc_t *desc = &request_descs[r->type];
  _message_serialize(&r->u, desc, _stdout_print_cb, NULL);
  printf("\n");
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

  /* serialization */
  const message_desc_t *desc = &response_descs[r->type];
  return _message_serialize(&r->u, desc, out, out_ctx);
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
  /* deserializes the JSON object from the input callback */
  json_t *json = json_load_callback(in, in_ctx, 0, NULL);
  if (json == NULL) {
    return false;
  }

  /* gets the request type */
  json_t *type = json_object_get(json, MSG_TYPE_KEY);
  if (type == NULL || json_typeof(type) != JSON_STRING) {
    json_decref(json);
    return false;
  }

  /* gets the coresponding description */
  r->type = response_last;
  const char *type_cstr = json_string_value(type);
  for (response_type_t t = 0; t < response_last; t++) {
    if (strcmp(type_cstr, response_descs[t].name) == 0) {
      r->type = t;
      break;
    }
  }

  if (r->type == response_last) {
    json_decref(json);
    return false;
  }

  /* deserializes from the JSON object */
  const message_desc_t *desc = &response_descs[r->type];
  bool success = _message_from_json(&r->u, desc, json);

  json_decref(json);
  return success;
}

/**
 * @brief Prints a response through STDOUT.
 *
 * @param r Response to print.
 */
void response_print(const response_t *r) {
  /* serialization */
  const message_desc_t *desc = &response_descs[r->type];
  _message_serialize(&r->u, desc, _stdout_print_cb, NULL);
  printf("\n");
}
