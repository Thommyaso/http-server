#ifndef __PARSER__
#define __PARSER__

#include "../network/connection.h"
#include "../utils/buff.h"

#define REQ_NOT_COMPLETE 1

#define REQ_COMPLETE 0

#define REQ_FAIL -1

#define LINE_END "\r\n"

#define STATUS_LINE_DELIMITER ' '

#define HEADER_END "\r\n\r\n"

#define HEADER_DELIMITER ':'

#define HEADER_LIMIT 50

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

typedef struct {
    char *label;
    char *value;
} label_val;

typedef struct {
    int header_count;
    char *method;
    char *url;
    char *http;
    label_val headers[HEADER_LIMIT];
} headers_map_t;

int parse_req(buff_t *buff, headers_map_t *header_map);

char *parse_line_section(char *data, headers_map_t *status_line);

int parse_headers_section(buff_t *buff, char *header_pos, headers_map_t *header_map);

#endif
