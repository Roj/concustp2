#define XCONCAT(a, b) a##b
#define XCONCAT3(a, b, c) a##b##c
#define XCONCAT4(a, b, c, d) a##b##c##d

#define CONCAT(a, b) XCONCAT(a, b)
#define CONCAT3(a, b, c) XCONCAT3(a, b, c)
#define CONCAT4(a, b, c, d) XCONCAT4(a, b, c, d)

/** Message types
 *  Defines an enum for each message (using the message name). The enums are named "MESSAGE_NAME + _ +
 * <item_name>""
 */
#define FIELD(...)
#define ENTRY(name, ...) CONCAT3(MESSAGE_NAME, _, name),

typedef enum { MESSAGES CONCAT(MESSAGE_NAME, _last) } CONCAT(MESSAGE_NAME, _type_t);

#undef ENTRY
#undef FIELD

/**
 * @brief Request structs definitions.
 * Defines a struct containing a field of the type specified in REQUESTS
 */
#define FIELD(struct_name, field_name, field_type) field_type##_t field_name;
#define ENTRY(name, fields)                                                                                  \
  typedef struct {                                                                                           \
    fields(name##_t)                                                                                         \
  } CONCAT4(MESSAGE_NAME, _, name, _t);

MESSAGES

#undef ENTRY
#undef FIELD

/**
 * @brief Array of field_desc structs { type, offset } for each message struct.
 */
#define FIELD(struct_name, field_name, field_type)                                                           \
  {.type = field_type_##field_type, .offset = offsetof(struct_name, field_name)},
#define ENTRY(name, fields)                                                                                  \
  static const struct field_desc CONCAT4(MESSAGE_NAME,name,_,fields_list)[] = {fields(CONCAT4(MESSAGE_NAME, _, name, _t))};

MESSAGES

#undef ENTRY
#undef FIELD

/**
 * @brief Array of message_desc used to iterate the structs (indexed per message type).
 */
#define FIELD(struct_name, field_name, field_type)
#define ENTRY(name, ...)                                                                                     \
  [CONCAT3(MESSAGE_NAME, _, name)] = {.num_fields = ASIZE(CONCAT4(MESSAGE_NAME, name, _, fields_list)),      \
                                      .fields = CONCAT4(MESSAGE_NAME, name, _, fields_list)},

static const struct message_desc CONCAT(MESSAGE_NAME, _descs)[] = {MESSAGES};

#undef ENTRY
#undef FIELD

/**
 * @brief Generic message container type.
 *  Contains a union with all the message structs and a field specifying the message type.
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
