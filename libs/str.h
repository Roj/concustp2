#ifndef STR_H
#define STR_H

/* include area */
#include <stdbool.h>
#include <string.h>

/* string type */
typedef struct {
  char buffer[1024];
  size_t length;
} string_t;

bool str_init(string_t *s, const char *cstr);
size_t str_len(const string_t *s);
int str_cmp(const string_t *s1, const string_t *s2);
int cstr_cmp(const string_t *s, const char *cstr);
const char *str_to_cstr(const string_t *s);

#endif
