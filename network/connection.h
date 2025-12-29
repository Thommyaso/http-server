#ifndef CONNECTION_H
#define CONNECTION_H

#include "../utils/buff.h"
#include "./listener.h"
#include <stddef.h>
#include <poll.h>

#define LIS_FD_IDX 0
#define POLL_FD_LIMIT 10000
#define INF_TIM -1

void server_run(fd_t lis_sock_fd);

void kill_client_connection(
    fd_t fd,
    struct pollfd pll_fds[],
    buff_t *preq_buff,
    res_buff_t *pres_buff
);

void handle_new_connection(
    fd_t lis_fd,
    struct pollfd poll_fds[],
    buff_t poll_requests[],
    res_buff_t poll_responses[],
    int *nfdsp1
);

void handle_client_request(
    struct pollfd *ppoll_fd,
    buff_t *preq_buff,
    res_buff_t *pres_buff
);

void handle_client_response(
    struct pollfd *ppoll_fd,
    buff_t *preq_buff,
    res_buff_t *pres_buff
);

#endif
