#include "../network/connection.h"

#define HEADER_END "\r\n\r\n"
#define REQ_NOT_COMPLETE 1
#define REQ_COMPLETE 0
#define REQ_FAIL -1

typedef struct {
    char *prot;
    char *url;
    char *storage;
} req_headers_t;

int parse_buf(req_buf_t *buf);
