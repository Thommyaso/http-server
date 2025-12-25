#include <stdlib.h>
#include <errno.h>
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
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    for(;;){
        // manual loop restart if interruptor occurs
        sock_fd_t conn_sock_fd;
        do{
            conn_sock_fd = accept(lis_sock_fd, (struct sockaddr *)&client_addr, &client_addr_size);
        }while(conn_sock_fd == -1 && errno == EINTR);

        int child_fork;
        if((child_fork = fork()) == 0){
            close(lis_sock_fd);

            buff_t req_buff = {0};
            result = init_buff(&req_buff, 0);

            // this will be in a loop
            if(req_buff.size - req_buff.used <= 0) {
                buff_increase(&req_buff, BUFF_SIZE);
            }
            int recv_size = recv(conn_sock_fd, req_buff.data + req_buff.used, req_buff.size - req_buff.used, 0);
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
            send(conn_sock_fd, res_buff.data, res_buff.used, 0);
            kill_buff(&req_buff);
            kill_buff(&res_buff);

            exit(EXIT_SUCCESS);
        }

        close(conn_sock_fd);
    }

    return EXIT_SUCCESS;
}
