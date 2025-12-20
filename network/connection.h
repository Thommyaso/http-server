#ifndef CONNECTION_H
#define CONNECTION_H

#include "./listener.h"
#include <stddef.h>

typedef struct {
    size_t capacity;
    size_t used;
    char *data;
} req_buf_t;

int server_run(sock_fd_t lis_sock_fd);

#endif
