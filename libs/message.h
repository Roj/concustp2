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

/**
 * @brief Message iteration callback.
 * This callback is called for every field in the message struct (like map).
 *
 * The first argument is a pointer to the field currently being iterated.
 * The second argument is the type of that field (so it can be casted properly)
 * The third argument is the same pointer passed to the message_iter function.
 *
 * If the return of this callback is false, then no further fields are iterated.
 */
typedef bool (*iter_cb_t)(void *field, field_type_t type, void *cb_ctx);
typedef bool (*const_iter_cb_t)(const void *field, field_type_t type, void *cb_ctx);

/** IO */
bool message_iter(void *message, const message_desc_t *desc, iter_cb_t cb, void *cb_ctx);
bool message_iter_const(const void *message, const message_desc_t *desc, const_iter_cb_t cb, void *cb_ctx);

#endif
