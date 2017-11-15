#include "message.h"
#include "scunit.h"
#include <stdbool.h>
#include <string.h>

/**
 * @brief Defines some messages.
 *
 */

// clang-format off

#define MSG1( )\
  ENTRY(type1,                             \
    FIELD(type1, bar, float))               \
  ENTRY(type2,                             \
    FIELD(type2, foo, string))

#define MSG2( )                            \
  ENTRY(ex1,                               \
    FIELD(ex1, integer, integer)           \
    FIELD(ex1, flt, float)               \
    FIELD(ex1, string, string))            \
  ENTRY(ex2,                               \
    FIELD(ex2, field, integer))

// clang-format on

#define MESSAGE_NAME msg1
#define MESSAGES MSG1( )

#include "message_decl.h"

#undef MESSAGES
#undef MESSAGE_NAME

/**
 * @brief Generates the response types.
 */
#define MESSAGE_NAME msg2
#define MESSAGES MSG2( )

#include "message_decl.h"

#undef MESSAGES
#undef MESSAGE_NAME

/**
 * @brief Helpers
 */

static bool _set_float(void *field, field_type_t type, void *cb_ctx) {
  if (type != field_type_float) {
    return false;
  }

  float_t *f = field;
  *f = 15.0;
  return true;
}

static bool _field_get(const void *field, field_type_t type, void *cb_ctx) {
  if (type != field_type_integer) {
    return false;
  }

  integer_t *output = cb_ctx;
  const integer_t *input = field;

  *output = *input;
  return true;
}

static bool _field_init(void *field, field_type_t type, void *cb_ctx) {
  switch (type) {
    case field_type_float:
      *(( float_t * )field) = 0.0;
      break;

    case field_type_integer:
      *(( integer_t * )field) = 1597;
      break;

    case field_type_string:
      str_init(field, cb_ctx);
      break;
  }
  return true;
}

TEST(MessageIter) {
  {
    msg1_t m = {0};
    m.type = msg1_type1;

    m.u.type1.bar = 3.1415;

    /* sets the float through the callback */
    ASSERT_TRUE(message_iter(&m.u, &msg1_descs[msg1_type1], _set_float, NULL));
    ASSERT_EQ(m.u.type1.bar, 15.0);
  }
  {
    msg1_t m = {0};
    m.type = msg1_type2;

    str_init(&m.u.type2.foo, "hello world");

    /* the callback expects a float */
    ASSERT_FALSE(message_iter(&m.u, &msg1_descs[msg1_type2], _set_float, NULL));
  }
  {
    msg2_t m = {0};
    m.type = msg2_ex1;

    ASSERT_TRUE(message_iter(&m.u, &msg2_descs[msg2_ex1], _field_init, "initialized string!"));
    ASSERT_EQ(m.u.ex1.flt, 0.0);
    ASSERT_EQ(m.u.ex1.integer, 1597);
    ASSERT_EQ(cstr_cmp(&m.u.ex1.string, "initialized string!"), 0);
  }
}

TEST(MessageIterConst) {
  msg2_t m = {0};
  m.type = msg2_ex2;
  m.u.ex2.field = 999;

  /* this value will be copied from the message through the iteration callback */
  integer_t obtained = 0;

  /* the callback expects a float */
  ASSERT_TRUE(message_iter_const(&m.u, &msg2_descs[msg2_ex2], _field_get, &obtained));
  ASSERT_EQ(obtained, 999);
}
