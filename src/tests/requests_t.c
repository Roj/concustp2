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

static size_t _read_cb(void *data, size_t bytes, void *cb_ctx) {
  buffer_t *buffer = cb_ctx;
  size_t bytes_to_copy = buffer->bytes - buffer->bytes_read;
  if (bytes_to_copy > bytes)
    bytes_to_copy = bytes;

  memcpy(data, buffer->data + buffer->bytes_read, bytes_to_copy);
  buffer->bytes_read += bytes_to_copy;
  return bytes_to_copy;
}

TEST(RequestSerialize) {
  {
    request_t r = {.type = request_currency};
    ASSERT_TRUE(str_init(&r.u.currency.currency, PESOS));

    buffer_t buffer = {0};
    ASSERT_TRUE(request_serialize(&r, _write_cb, &buffer));

    /* deserializes the request */
    request_t rd = {0};
    ASSERT_TRUE(request_deserialize(&rd, _read_cb, &buffer));

    /* compares the fields */
    ASSERT_EQ(r.type, rd.type);
    ASSERT_EQ(str_cmp(&r.u.currency.currency, &rd.u.currency.currency), 0);
  }
  {
    request_t r = {.type = request_weather};
    ASSERT_TRUE(str_init(&r.u.weather.city, BSAS));

    buffer_t buffer = {0};
    ASSERT_TRUE(request_serialize(&r, _write_cb, &buffer));

    /* deserializes the request */
    request_t rd = {0};
    ASSERT_TRUE(request_deserialize(&rd, _read_cb, &buffer));

    /* compares the fields */
    ASSERT_EQ(r.type, rd.type);
    ASSERT_EQ(str_cmp(&r.u.weather.city, &rd.u.weather.city), 0);
  }
}

TEST(ResponseSerialize) {
  {
    response_t r = {.type = response_weather};
    r.u.weather.humidity = 57;
    r.u.weather.pressure = 1.2;
    r.u.weather.temperature = 27.3;

    buffer_t buffer = {0};
    ASSERT_TRUE(response_serialize(&r, _write_cb, &buffer));

    /* deserializes the response */
    response_t rd = {0};
    ASSERT_TRUE(response_deserialize(&rd, _read_cb, &buffer));

    /* compares the fields */
    ASSERT_EQ(r.type, rd.type);
    ASSERT_EQ(r.u.weather.humidity, rd.u.weather.humidity);
    ASSERT_EQ(r.u.weather.pressure, rd.u.weather.pressure);
    ASSERT_EQ(r.u.weather.temperature, rd.u.weather.temperature);
  }
}
