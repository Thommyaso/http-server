#ifndef CONNECTION_H
#define CONNECTION_H

#include "../utils/buff.h"
#include "./listener.h"
#include <stddef.h>
#include <poll.h>

#define LIS_FD_IDX 0
#define POLL_FD_LIMIT 10000

typedef struct {
    size_t capacity;
    size_t used;
    char *data;
} req_buf_t;

int server_run(sock_fd_t lis_sock_fd);

void kill_client_connection(sock_fd_t fd, struct pollfd pll_fds[], buff_t *preq_buff, buff_t *pres_buff);

#endif
