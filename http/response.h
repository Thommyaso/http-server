#include "../router/router.h"

#define RESPONSE_FMT \
"HTTP/1.1 200 OK\r\n"\
"Content-Length: %d\r\n"\
"\r\n"

int build_success_res(resource_t *resource);
char *prepend(char *data, char *prefix);
