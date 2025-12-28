#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <poll.h>

#include "./listener.h"
#include "./connection.h"
#include "../http/parser.h"
#include "../http/response.h"
#include "../utils/buff.h"

#define LIS_FD_IDX 0
#define POLL_FD_LIMIT 10000

int server_run(sock_fd_t lis_sock_fd)
{
    int result;
    int nfdsp1 = lis_sock_fd + 1;
    int highest_fd = lis_sock_fd;

    struct pollfd poll_fds[POLL_FD_LIMIT];

    poll_fds[LIS_FD_IDX].fd = lis_sock_fd;
    poll_fds[LIS_FD_IDX].events = POLLRDNORM;

    buff_t poll_requests[POLL_FD_LIMIT];
    buff_t poll_responses[POLL_FD_LIMIT];


    for(int idx = 1; idx < POLL_FD_LIMIT; idx++){
         // -1 indicates available space for accepted fd to be put
        poll_fds[idx].fd = -1;
    }

    for(;;){
        int nready = poll(poll_fds, nfdsp1, -1);

        if(nready <= 0){
            // gonna have to read those errors and see what to do about them
            continue;
        }

        if (poll_fds[LIS_FD_IDX].revents & POLLRDNORM) {
            sock_fd_t conn_sock_fd = accept(lis_sock_fd, NULL, NULL);

            if(conn_sock_fd < 0){
                continue; // connection socket failed;
            }

            int should_close = 1;
            for (int idx = 1; idx < POLL_FD_LIMIT; idx++) {
                if(poll_fds[idx].fd == -1){
                    if(poll_requests[idx].data != NULL || poll_responses[idx].data != NULL){
                        // something went wrong, probably forgot to clean up req/res heap after finishing with the area
                        // we shouldn't arrive here unless idx indicates that this is a free position but the value is still a pointer to something
                        // just overwriting it could result in something else trying  to read it in the future (possibly not terminated correctly?)
                        // got to make sure that  after finishing connection both req and res pointers are reset to null in arrays (poll_requests and poll_responses)
                        continue;
                    }

                    result = init_buff(&poll_requests[idx], 0);
                    if(result){
                        // error allocating request buff memory
                        // it aint happening for this client, sorry
                        break;
                    }

                    result = init_buff(&poll_responses[idx], 0);
                    if(result){
                        // error allocating response buff memory
                        // it aint happening for this client, sorry
                        kill_buff(&poll_requests[idx]);
                        break;
                    }

                    poll_fds[idx].fd = conn_sock_fd;
                    poll_fds[idx].events = POLLRDNORM;

                    if(nfdsp1 <= idx){
                        nfdsp1 = idx + 1;
                    }

                    should_close = 0;
                    break;
                }
            }

            // max number of file descriptors in reading set has been reached or error initiating buffer for req/response,
            // cant do any more connections till space frees
            if(should_close){
                close(conn_sock_fd);
            }

            if(--nready == 0){
                continue; 
            }
        }

        for(int idx = 1; idx < POLL_FD_LIMIT && nready > 0; idx++){
            int fd = poll_fds[idx].fd;
            if(fd < 0){
                continue;
            }

            if (poll_fds[idx].revents & POLLERR) {
                printf("error nr:\n", errno);
                nready--;
            }

            if (poll_fds[idx].revents & POLLRDNORM) {
                nready--;
                buff_t *req_buff = &poll_requests[idx];
                buff_t *res_buff = &poll_responses[idx];

                if(req_buff->data == NULL || res_buff->data == NULL){
                    // for some reason the req/res buff is not initiated for this client
                    // this shouldn't happen but if it does, close connection and bye
                    close(fd);
                    poll_fds[idx].fd = -1;
                    break;
                }

                // this will be in a loop
                if(req_buff->size - req_buff->used <= 0) {
                    buff_increase(req_buff, BUFF_SIZE);
                }

                int recv_size = recv(fd, req_buff->data + req_buff->used, req_buff->size - req_buff->used, 0);
                req_buff->used += recv_size;
                req_buff->data[req_buff->used] = '\0';

                headers_map_t headers_map = {0};
                result = parse_req(req_buff, &headers_map);
                //

                result = build_response(&headers_map, res_buff);

                // this needs to loop (in case all data wasn't sent)
                send(fd, res_buff->data, res_buff->used, 0);

                kill_buff(req_buff);
                kill_buff(res_buff);
                //

                close(fd);
                poll_fds[idx].fd = -1; // getting rid of the descriptor
            }
        }
    }

    return EXIT_SUCCESS;
}
