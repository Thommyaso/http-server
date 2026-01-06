#ifndef __RESPONSE__
#define __RESPONSE__

#include "router.h"
#include "utils/buff.h"
#include "parser.h"

#define RESPONSE_FMT \
"HTTP/1.1 %d DO NOT CARE\r\n"\
"Content-Length: %zu\r\n"\
"\r\n"

int build_response(headers_map_t *headers_map, res_buff_t *res_buff);
#endif
