/* include area */
#include "str.h"

/**
 * @brief Initializes a string_t from a C string.
 *
 * @param s String to initialize.
 * @param cstr C string to copy into s.
 * @return false if the C string was too large, true on success.
 */
bool str_init(string_t *s, const char *cstr) {
  size_t length = strlen(cstr);
  if (length >= sizeof(s->buffer))
    return false;

  s->length = length;
  memcpy(s->buffer, cstr, length);
  s->buffer[s->length] = '\0';
  return true;
}

/**
 * @brief Returns the length of a string.
 *
 * @param s String.
 * @return length of the string.
 */
size_t str_len(const string_t *s) {
  return s->length;
}

/**
 * @brief Compares 2 strings.
 *
 * @param s1
 * @param s2
 * @return   0  if both strings are equal.
 *         < 0 if s2 > s1
 *         > 0 if s1 > s2
 */
int str_cmp(const string_t *s1, const string_t *s2) {
  if (str_len(s1) != str_len(s2))
    return str_len(s1) - str_len(s2);

  return memcmp(s1->buffer, s2->buffer, s1->length);
}

/**
 * @brief Compares a string with a C string.
 *
 * @param s string.
 * @param cstr C string.
 * @return   0  if both strings are equal.
 *         < 0 if cstr > s
 *         > 0 if s > cstr
 */
int cstr_cmp(const string_t *s, const char *cstr) {
  size_t length = strlen(cstr);
  if (str_len(s) != length)
    return str_len(s) - length;

  return memcmp(s->buffer, cstr, length);
}

/**
 * @brief Returns the C string represantion.
 *
 * @param s String.
 * @return C string.
 */
const char *str_to_cstr(const string_t *s) {
  return s->buffer;
}