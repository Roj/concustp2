#include "requests.h"
#include "scunit.h"
#include <stdbool.h>
#include <string.h>

#define PESOS "pesos"
#define BSAS "Buenos Aires"
#define SE "Santiago del Estero"

typedef struct {
  size_t bytes;
  size_t bytes_read;
  char data[10 << 10];
} buffer_t;

static bool _write_cb(const void *data, size_t bytes, void *cb_ctx) {
  buffer_t *buffer = cb_ctx;
  if (buffer->bytes + bytes > sizeof(buffer->data))
    return false;

  memcpy(buffer->data + buffer->bytes, data, bytes);
  buffer->bytes += bytes;
  return true;
}

static bool _read_cb(void *data, size_t bytes, void *cb_ctx) {
  buffer_t *buffer = cb_ctx;
  if (bytes > sizeof(buffer->data) - buffer->bytes_read)
    return false;

  memcpy(data, buffer->data + buffer->bytes_read, bytes);
  buffer->bytes_read += bytes;
  return true;
}

TEST(RequestSerialize) {
  {
    request_t r = {.type = req_currency};
    ASSERT_TRUE(str_init(&r.u.currency.currency, PESOS));

    buffer_t buffer = {0};
    ASSERT_TRUE(request_serialize(&r, _write_cb, &buffer));

    /* checks the serialized size: 1 + 4 + 5 (type + field size + content) */
    ASSERT_EQ(buffer.bytes, 10);

    /* checks the type */
    ASSERT_EQ(buffer.data[0], req_currency);

    /* checks the field size */
    ASSERT_EQ(memcmp(buffer.data + 1, "0005", MAX_SIZE_IN_BYTES), 0);

    /* checks the field content */
    ASSERT_EQ(memcmp(buffer.data + 5, PESOS, 5), 0);
  }
  {
    request_t r = {.type = req_weather};
    ASSERT_TRUE(str_init(&r.u.weather.city, BSAS));

    buffer_t buffer = {0};
    ASSERT_TRUE(request_serialize(&r, _write_cb, &buffer));

    /* checks the serialized size: 1 + 4 + 12 (type + field size + content) */
    ASSERT_EQ(buffer.bytes, 17);

    /* checks the type */
    ASSERT_EQ(buffer.data[0], req_weather);

    /* checks the field size */
    ASSERT_EQ(memcmp(buffer.data + 1, "0012", MAX_SIZE_IN_BYTES), 0);

    /* checks the field content */
    ASSERT_EQ(memcmp(buffer.data + 5, BSAS, 5), 0);
  }
}

TEST(RequestDeserialize) {
  {
#define DATA                                                                                                 \
  "\x01"                                                                                                     \
  "0007"                                                                                                     \
  "dollars"

    request_t r;
    buffer_t buffer = {0};
    buffer.bytes = sizeof(DATA) - 1;
    memcpy(buffer.data, DATA, sizeof(DATA));

    ASSERT_TRUE(request_deserialize(&r, _read_cb, &buffer));
    ASSERT_EQ(buffer.bytes_read, buffer.bytes);

    ASSERT_EQ(r.type, req_currency);
    ASSERT_EQ(cstr_cmp(&r.u.currency.currency, "dollars"), 0);

#undef DATA
  }
  {
#define DATA                                                                                                 \
  "\x00"                                                                                                     \
  "0019" SE

    request_t r;
    buffer_t buffer = {0};
    buffer.bytes = sizeof(DATA) - 1;
    memcpy(buffer.data, DATA, sizeof(DATA));

    ASSERT_TRUE(request_deserialize(&r, _read_cb, &buffer));
    ASSERT_EQ(buffer.bytes_read, buffer.bytes);

    ASSERT_EQ(r.type, req_weather);
    ASSERT_EQ(cstr_cmp(&r.u.weather.city, SE), 0);

#undef DATA
  }
}

TEST(ResponseSerialize) {
  {
    response_t r = {.type = resp_weather};
    r.u.weather.humidity = 57;
    r.u.weather.pressure = 1.2;
    r.u.weather.temperature = 27.3;

    buffer_t buffer = {0};
    ASSERT_TRUE(response_serialize(&r, _write_cb, &buffer));

    /* checks the serialized size: 1 + 4 + 2 + 4 + 6 + 4 + 7 */
    ASSERT_EQ(buffer.bytes, 28);

    /* checks the type */
    ASSERT_EQ(buffer.data[0], resp_weather);

    /* checks the field size */
    ASSERT_EQ(memcmp(buffer.data + 1, "0002", MAX_SIZE_IN_BYTES), 0);

    /* checks the field content */
    ASSERT_EQ(memcmp(buffer.data + 5, "57", 2), 0);

    /* checks the other fields */
    ASSERT_EQ(memcmp(buffer.data + 7, "0006", MAX_SIZE_IN_BYTES), 0);
    ASSERT_EQ(memcmp(buffer.data + 11, "1.2000", 2), 0);
    ASSERT_EQ(memcmp(buffer.data + 17, "0007", MAX_SIZE_IN_BYTES), 0);
    ASSERT_EQ(memcmp(buffer.data + 21, "27.3000", 2), 0);
  }
}
