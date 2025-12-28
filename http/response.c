#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include "../utils/buff.h"
#include "response.h"
#include "parser.h"

#define MAX_HEADER 500

int build_response(headers_map_t *headers_map, buff_t *res_buff)
{
    char header[MAX_HEADER] = {0}; 

    // this is important to reset, just in case
    res_buff->processed = 0;
    res_buff->used = 0;

    // send failed response, we only accept GET requests
    if(strcmp(headers_map->method, "GET") != 0) return 1;

    // send failed response, no url path
    if(strlen(headers_map->url) <= 0) return 1;

    res_code_t res_code = get_resource(res_buff, headers_map);

    // at this stage the only thing in respose buffer is the content, so das ist gut
    snprintf(header, sizeof(header), RESPONSE_FMT, res_code, res_buff->used);

    int result = buff_prepend(res_buff, header);
    return result;
}
