#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../network/connection.h"
#include "./parser.h"

int parse_buf(req_buf_t *buf)
{
    char *header_end = strstr(buf->data, HEADER_END);
    if(header_end == NULL){
        //header not complete tell connnection to keep waiting for rest of the info
        return REQ_NOT_COMPLETE;

    }

    if(buf->used < 10){
        // maybe this is wrong but i assume that http request needs to have at least 10 bytes to successfully store the status line
        // I'll fix it later if needed
        return REQ_FAIL;
    }

    char *prot_end = strchr(buf->data, ' ');
    size_t prot_len = (size_t)(prot_end - buf->data);

    char *url_start = prot_end + 1;
    char *url_end = strchr(url_start, ' ');
    size_t url_len = (size_t)(url_end - url_start);

    size_t t_storage = prot_len + 1 + url_len + 1;

    req_headers_t req_headers;
    req_headers.storage = malloc( t_storage * sizeof(*req_headers.storage));

    char *p = req_headers.storage;

    memcpy(p, buf->data, prot_len);
    req_headers.prot = p;
    p += prot_len;
    *p++ = '\0';

    memcpy(p, url_start, url_len);
    req_headers.url = p;
    p += url_len;
    *p++ = '\0';

    printf("Protocol: %s\n", req_headers.prot);
    printf("Url: %s\n", req_headers.url);

    if(strcmp(req_headers.prot, "GET") != 0){
        // Atm im only handling get requests, everything else can fuck off
        return REQ_FAIL;
    }

    return REQ_COMPLETE;
}
