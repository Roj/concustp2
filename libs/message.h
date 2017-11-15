#ifndef MESSAGE_H
#define MESSAGE_H

/**
 * @brief This module handles generic messages.
 * The messages should be declared by including "message_decl.h" (as explained
 * in that header).
 *
 * The generated messages can then be used with this module's functions.
 */

/* include area */
#include "types.h"
#include <stddef.h>

/** The number of bytes used to serialize the field size */
#define MAX_SIZE_IN_BYTES 4

/*--------------------------------------------------------------------------
   Utils
--------------------------------------------------------------------------*/

/** Returns the number of elements in an *array* (doesn't work on pointers!!!) */
#define ASIZE(array) (sizeof(array) / sizeof(array[0]))

/*--------------------------------------------------------------------------
   Types
--------------------------------------------------------------------------*/

/** Field description. */
struct field_desc {
  /** Field type */
  field_type_t type;
  /** offset (in bytes) of the field from the beggining of the message struct */
  size_t offset;
};

/** Message description. */
typedef struct message_desc {
  /** Array of field descriptions (one for each field in the message struct). */
  const struct field_desc *fields;
  /** Number of fields (i.e. elements in the array "fields"). */
  size_t num_fields;
} message_desc_t;

/*--------------------------------------------------------------------------
   Prototypes
--------------------------------------------------------------------------*/

/** Callback where a message is read from */
typedef bool (*read_cb_t)(void *output, size_t bytes, void *cb_ctx);

/** Callback where a message is written to */
typedef bool (*write_cb_t)(const void *data, size_t bytes, void *cb_ctx);

/** IO */
bool message_serialize(const void *message, const message_desc_t *desc, write_cb_t out, void *out_ctx);
bool message_deserialize(void *message, const message_desc_t *desc, read_cb_t in, void *in_ctx);

#endif
