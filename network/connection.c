#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "./listener.h"
#include "./connection.h"
#include "../http/parser.h"
#include "../router/router.h"
#include "../http/response.h"
#include "../utils/buff.h"


int server_run(sock_fd_t lis_sock_fd)
{
    int result;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    sock_fd_t conn_sock_fd = accept(lis_sock_fd, (struct sockaddr *)&client_addr, &client_addr_size);

    buff_t req_buff;
    result = init_buff(&req_buff, 0);

    // this will be in a loop
    if(req_buff.size - req_buff.used <= 0) {
        buff_increase(&req_buff, BUFF_SIZE);
    }
    int recv_size = recv(conn_sock_fd, req_buff.data + req_buff.used, req_buff.size - req_buff.used, 0);
    req_buff.used = recv_size;
    
    int req = parse_req(&req_buff);
    if(req != REQ_COMPLETE){
    }
    //
   
    buff_t res_buff;
    result = init_buff(&res_buff, 0);
    if(result != 0){
        // oh no malloc failed!
        return result;
    }

    result = get_resource(&res_buff);
    result = build_success_res(&res_buff);

    // this needs to loop (in case all data wasn't sent)
    send(conn_sock_fd, res_buff.data, res_buff.used, 0);
    // shutdown(conn_sock_fd, SHUT_RDWR); 

    printf("%s\n", res_buff.data);
    return EXIT_SUCCESS;
}
