#include "server.h"
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int launch_microservice(request_type_t type, bool* exit_flag);
// Returns the basic request_type associated with the provided request.
// i.e.,
// request_post_weather => request_weather
// request_weather => request_weather
// etc.
// Useful for rerouting requests and obtaining the port associated with the microservice.
request_type_t get_base_request(request_type_t type);
