/* include area */
#include "message.h"

/**
 * @brief Iterates through the fields of a message applying the given callback.
 *
 * @param message Pointer to the message (the structs in the union, not the group!).
 * @param desc The corresponding message's field descriptions.
 * @param cb The callback applied to each field.
 * @param cb_ctx Pointer passed to the cb.
 * @return false If the cb returned false. true if all fields where iterated.
 */
bool message_iter(void *message, const message_desc_t *desc, iter_cb_t cb, void *cb_ctx) {
  /* iterates through the message fields applying the callback on them */
  for (size_t i = 0; i < desc->num_fields; i++) {
    void *field = (( uint8_t * )message) + desc->fields[i].offset;
    if (!cb(field, desc->fields[i].type, cb_ctx)) {
      return false;
    }
  }

  return true;
}

/**
 * @brief Same as message_iter but with a const pointer to a message.
 *
 * @param message Pointer to the message (the structs in the union, not the group!).
 * @param desc The corresponding message's field descriptions.
 * @param cb The callback applied to each field.
 * @param cb_ctx Pointer passed to the cb.
 * @return false If the cb returned false. true if all fields where iterated.
 */
bool message_iter_const(const void *message, const message_desc_t *desc, const_iter_cb_t cb, void *cb_ctx) {
  /* iterates through the message fields applying the callback on them */
  for (size_t i = 0; i < desc->num_fields; i++) {
    void *field = (( uint8_t * )message) + desc->fields[i].offset;
    if (!cb(field, desc->fields[i].type, cb_ctx)) {
      return false;
    }
  }

  return true;
}
