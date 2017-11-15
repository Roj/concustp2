/**
 * @brief This file works as a template.
 * Before including it, the MESSAGES and MESSAGE_NAME macros should be defined
 * so this file generates the appropriate message structures.
 *
 * Messages are organized by groups (MESSAGES).
 * Each message is declared with a macro "ENTRY" (do not define it yourself!!!)
 * ENTRY will receive as first argument the message name, and then any number of
 * fields.
 *
 * The fields are declared with the "FIELD" macro (again, do not define it yourself!!!)
 * The FIELD macro receives 3 parameters:
 *   * the message name (the one specified in ENTRY)
 *   * the field name
 *   * the field type (any of the fields in types.h)
 *
 * For example:
 *
 *    // Declare a group of messages named foobar
 *    #define MESSAGE_NAME foobar
 *
 *    // Declare the messages and their fields
 *    #define FOOBAR()                \
 *      ENTRY(foo,                    \
 *        FIELD(foo, bar, float))     \
 *      ENTRY(foo2,                   \
 *        FIELD(foo2, fld, integer))  \
 *        FIELD(foo2, oth, string))
 *
 *    // Define the MESSAGE group macro
 *    #define MESSAGES FOOBAR()
 *
 *    // include the template
 *    #include "message_decl.h"
 *
 *    // undefine and repeat as many times as needed
 *    #undef MESSAGE_NAME
 *    #undef MESSAGES
 *
 * defines a group "foobar" with 2 messages: "foo" and "foo2"
 *   * foo has only 1 field named "bar" of type float_t
 *   * foo2 has 2 fields named "fld" and "oth" of type integer_t and string_t
 *
 * The resulting message group type will look something like this:
 *
 *    typedef struct {
 *      foobar_type_t type;
 *      union {
 *        foobar_foo_t foo;
 *        foobar_foo2_t foo2;
 *      } u;
 *    } foobar_t;
 *
 * and the message types (for each ENTRY):
 *
 *    typedef struct {
 *      integer_t fld;
 *      string_t oth;
 *    } foobar_foo2_t;
 */

/** Utility macros to expand concatenate tokens */
#define XCONCAT(a, b) a##b
#define XCONCAT3(a, b, c) a##b##c
#define XCONCAT4(a, b, c, d) a##b##c##d

#define CONCAT(a, b) XCONCAT(a, b)
#define CONCAT3(a, b, c) XCONCAT3(a, b, c)
#define CONCAT4(a, b, c, d) XCONCAT4(a, b, c, d)

/**
 * @brief  Message types
 *  Defines an enum for each message (using the message name). The enums are named "MESSAGE_NAME + _ +
 * <item_name>""
 *
 * For example, having a list of MESSAGE_NAME "foobar" [foo, foo2, foo3]:
 *
 *   typedef enum {
 *     foobar_foo,
 *     foobar_foo2,
 *     foobar_foo3,
 *     foobar_last
 *   } foobar_type_t;
 *
 */
#define FIELD(...)
#define ENTRY(name, ...) CONCAT3(MESSAGE_NAME, _, name),

typedef enum { MESSAGES CONCAT(MESSAGE_NAME, _last) } CONCAT(MESSAGE_NAME, _type_t);

#undef ENTRY
#undef FIELD

/**
 * @brief Message structs definitions.
 * Defines a struct containing a field of the type specified in MESSAGES
 *
 * For example, having a list
 *
 * #define MESSAGES               \
 *    ENTRY(foo,                   \
 *      FIELD(foo, bar, float)     \
 *      FIELD(foo, bar2, integer))
 *
 * and assuming MESSAGE_NAME is "foobar" would result in:
 *
 *   typedef struct {
 *     float_t bar;
 *     integer_t bar2;
 *   } foobar_foo_t;
 */
#define FIELD(msg_name, field_name, field_type) field_type##_t field_name;
#define ENTRY(name, ...)                                                                                     \
  typedef struct { __VA_ARGS__ } CONCAT4(MESSAGE_NAME, _, name, _t);

MESSAGES

#undef ENTRY
#undef FIELD

/**
 * @brief Array of field_desc structs { type, offset } for each message struct.
 * Continuing the previous example:
 *
 *    static const struct field_desc foobar_foo_fields_list[] = {
 *      { .type = field_type_float, .offset = offsetof( foobar_foo_t, bar ) },
 *      { .type = field_type_integer, .offset = offsetof( foobar_foo_t, bar2 ) },
 *    };
 */
#define FIELD(msg_name, field_name, field_type)                                                              \
  {.type = field_type_##field_type, .offset = offsetof(CONCAT4(MESSAGE_NAME, _, msg_name, _t), field_name)},
#define ENTRY(name, ...)                                                                                     \
  static const struct field_desc CONCAT4(MESSAGE_NAME, name, _, fields_list)[] = {__VA_ARGS__};

MESSAGES

#undef ENTRY
#undef FIELD

/**
 * @brief Array of message_desc used to iterate the structs, indexed per message type (the enum).
 * Continuing the previous example:
 *
 *    static const struct message_desc foobar_descs[] = {
 *      [foobar_foo] = { .num_fields = ASIZE( foobar_foo_fields_list ), .fields = foobar_foo_fields_list }
 *    };
 */
#define FIELD(msg_name, field_name, field_type)
#define ENTRY(name, ...)                                                                                     \
  [CONCAT3(MESSAGE_NAME, _, name)] = {.num_fields = ASIZE(CONCAT4(MESSAGE_NAME, name, _, fields_list)),      \
                                      .fields = CONCAT4(MESSAGE_NAME, name, _, fields_list)},

static const struct message_desc CONCAT(MESSAGE_NAME, _descs)[] = {MESSAGES};

#undef ENTRY
#undef FIELD

/**
 * @brief Generic message container type.
 *  Contains a union with all the message structs and a field specifying the message type.
 *
 *    typedef struct {
 *      foobar_type_t type;  // the enum indicating what struct in the union was used
 *      union {
 *        foobar_foo_t foo;  // and any other "foobar" defined in MESSAGES
 *      } u;
 *    } foobar_t;
 */
#define FIELD(...)
#define ENTRY(name, ...) CONCAT4(MESSAGE_NAME, _, name, _t) name;

typedef struct {
  /** The type of the message instance. */
  CONCAT(MESSAGE_NAME, _type_t) type;
  /** Message contents (should use the union field that corresponds to "type").  */
  union {
    MESSAGES
  } u;
} CONCAT(MESSAGE_NAME, _t);

#undef ENTRY
#undef FIELD
