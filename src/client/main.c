/* include area */
#include "client.h"
#include "str.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define SERVER_PORT 8002

int main(int argc, const char *argv[]) {
  /* initializes the request */
  request_t req = {0};
  srand(time(NULL));
  if (rand() % 2 == 1) {
    char* ciudades[] = {"buenos aires", "bahia blanca", "comodo rivadavia"};
    char* ciudad = ciudades[rand() % 3];
    req.type = request_weather;
    str_init(&req.u.weather.city, ciudad);
  } else {
    char* monedas[] = {"peso", "euro", "dollar"};
    char* moneda = monedas[rand() % 3];
    req.type = request_currency;
    str_init(&req.u.currency.currency, moneda);
  }

  response_t resp = {0};
  if (!client_send(&resp, SERVER_PORT, &req)) {
    perror("Failed sending the request");
    return 1;
  }

  // TODO: handle all responses
  switch (resp.type) {
    case response_currency:
      printf("Currency response: %f\n", resp.u.currency.quote);
      break;
    case response_weather:
      printf("Weather response: %d humid, %f temp, %f press\n",
        resp.u.weather.humidity, resp.u.weather.temperature, resp.u.weather.pressure);
      break;
    default:
      break;
  }

  return 0;
}
