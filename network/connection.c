#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include "./listener.h"
#include "./connection.h"
#include "../http/parser.h"
#include "../http/response.h"
#include "../utils/buff.h"

int server_run(sock_fd_t lis_sock_fd)
{
    int result;
    int clients[FD_SETSIZE] = {0};
    int nfdsp1 = lis_sock_fd + 1;
    fd_set local_fd_set, read_fd_set;
    FD_ZERO(&local_fd_set);
    FD_SET(lis_sock_fd, &local_fd_set);

    for(int idx = 0; idx < FD_SETSIZE; idx++)
        clients[idx] = -1; // -1 indicates available space for accepted fd to be put

    for(;;){
        read_fd_set = local_fd_set;
        int nready = select(nfdsp1, &read_fd_set, NULL, NULL, NULL);
        if(nready < 0){
            return EXIT_FAILURE;
        }

        if(FD_ISSET(lis_sock_fd, &read_fd_set)){
            sock_fd_t conn_sock_fd = accept(lis_sock_fd, NULL, NULL);

            if(conn_sock_fd < 0)
                continue; // connection socket failed;

            int idx;
            for (idx = 0; idx < FD_SETSIZE; idx++) {
                if(clients[idx] == -1){
                    clients[idx] = conn_sock_fd;
                    break;
                }
            }

            if(idx < FD_SETSIZE){
                FD_SET(conn_sock_fd, &local_fd_set);

                if(nfdsp1 <= conn_sock_fd){
                    nfdsp1 = conn_sock_fd + 1;
                }

            } else {
                // max number of file descriptors in reading set has been reached, cant do any more connections till space frees
                close(conn_sock_fd);
            }

            nready--;
            if(nready == 0){
                continue; // the only fd that was ready was the listener with a new connection that needs setting;
            }
        }

        for(int idx = 0; idx <= FD_SETSIZE; idx++){
            int descriptor = clients[idx];
            if(descriptor < 0){
                continue;
            }

            if(FD_ISSET(descriptor, &read_fd_set)){
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
                FD_CLR(descriptor, &local_fd_set);
                clients[idx] = -1;

                nready--;
                if(nready <= 0){
                    break;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}
