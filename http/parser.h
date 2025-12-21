#include "../network/connection.h"

#define HEADER_END "\r\n\r\n"
#define REQ_NOT_COMPLETE 1
#define REQ_COMPLETE 0
#define REQ_FAIL -1

/**
 * Parsed request headers.
 *
 * @var prot:
 * Non-owning, null-terminated protocol string.
 * Points into storage.
 *
 * @var url:
 * Non-owning, null-terminated URL string.
 * Points into storage.
 *
 * @var storage:
 * Owning heap allocation backing all fields.
 * Must be freed.
 */
typedef struct {
    char *prot;
    char *url;
    char *storage;
} req_headers_t;

int parse_req(req_buf_t *buf);
