#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include "../utils/buff.h"
#include "response.h"
#include "parser.h"

#define TMP_BUFF_SIZE 500

int build_response(headers_map_t *headers_map, res_buff_t *res_buff)
{
    int fail;
    char tmp_buff[TMP_BUFF_SIZE] = {0}; 

    // this is important to reset, just in case
    res_buff->base.size_processed = 0;
    res_buff->base.size_used = 0;

    // send failed response, we only accept GET requests
    if(strcmp(headers_map->method, "GET") != 0) return 1;

    // send failed response, no url path
    if(strlen(headers_map->url) <= 0) return 1;

    res_code_t res_code = find_resource(res_buff, headers_map);

    snprintf(tmp_buff, sizeof(tmp_buff),  "HTTP/1.1 %d DO NOT CARE\r\n", res_code);
    fail = buff_append(&res_buff->base, tmp_buff);
    if(fail) return 1;

    snprintf(tmp_buff, sizeof(tmp_buff),  "Content-Length: %zu\r\n", res_buff->filesize);
    fail = buff_append(&res_buff->base,tmp_buff);
    if(fail) return 1;

    // empty line indicating header end (since previous header already has "\r\n" at the end of it)
    fail = buff_append(&res_buff->base, "\r\n");
    return fail;
}
