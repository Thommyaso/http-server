#include "../router/router.h"
#include "../utils/buff.h"

#define RESPONSE_FMT \
"HTTP/1.1 200 OK\r\n"\
"Content-Length: %d\r\n"\
"\r\n"

int build_success_res(buff_t *resource);
