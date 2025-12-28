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

    struct pollfd fds[POLL_FD_LIMIT];

    fds[LIS_FD_IDX].fd = lis_sock_fd;
    fds[LIS_FD_IDX].events = POLLIN;

    for(int idx = 1; idx < POLL_FD_LIMIT; idx++){
         // -1 indicates available space for accepted fd to be put
        fds[idx].fd = -1;
    }

    for(;;){
        int nready = poll(fds, nfdsp1,0);

        if(nready < 0){
            return EXIT_FAILURE;
        }

        if (fds[LIS_FD_IDX].revents & POLLIN) {
            sock_fd_t conn_sock_fd = accept(lis_sock_fd, NULL, NULL);

            if(conn_sock_fd < 0)
                continue; // connection socket failed;

            int idx;
            for (idx = 1; idx < POLL_FD_LIMIT; idx++) {
                if(fds[idx].fd == -1){
                    fds[idx].fd = conn_sock_fd;
                    fds[idx].events = POLLIN | POLLOUT;

                    if(nfdsp1 <= conn_sock_fd){
                        nfdsp1 = conn_sock_fd + 1;
                    }
                    break;
                }
            }

            // max number of file descriptors in reading set has been reached, cant do any more connections till space frees
            if(idx >= POLL_FD_LIMIT){
                close(conn_sock_fd);
            }

            // if true the only fd that was ready was the listener
            if(--nready == 0){
                continue; 
            }
        }

        for(int idx = 1; idx <= POLL_FD_LIMIT; idx++){
            int descriptor = fds[idx].fd;
            if(descriptor < 0){
                continue;
            }

            if (fds[idx].revents & POLLIN) {
                buff_t req_buff = {0};
                result = init_buff(&req_buff, 0);

                // this will be in a loop
                if(req_buff.size - req_buff.used <= 0) {
                    buff_increase(&req_buff, BUFF_SIZE);
                }

                int recv_size = recv(descriptor, req_buff.data + req_buff.used, req_buff.size - req_buff.used, 0);
                req_buff.used += recv_size;
                req_buff.data[req_buff.used] = '\0';

                headers_map_t headers_map = {0};
                result = parse_req(&req_buff, &headers_map);
                //

                buff_t res_buff = {0};
                result = init_buff(&res_buff, 0);

                // oh no malloc failed!
                if(result != 0) return result;

                result = build_response(&headers_map, &res_buff);

                // this needs to loop (in case all data wasn't sent)
                send(descriptor, res_buff.data, res_buff.used, 0);
                kill_buff(&req_buff);
                kill_buff(&res_buff);

                close(descriptor);
                fds[idx].fd = -1; // getting rid of the descriptor

                if(--nready <= 0){
                    break;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}
