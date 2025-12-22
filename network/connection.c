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
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    sock_fd_t conn_sock_fd = accept(lis_sock_fd, (struct sockaddr *)&client_addr, &client_addr_size);

    req_buf_t req_buf = {
        .capacity = 2000,
        .used = 0,
    };
    req_buf.data = malloc(req_buf.capacity * sizeof(*req_buf.data));

    if(req_buf.data == NULL){
        return EXIT_FAILURE;
    }

    // this will be in a loop
    int recv_size = recv(conn_sock_fd, req_buf.data, req_buf.capacity, 0);
    req_buf.used = recv_size;
    
    int req = parse_req(&req_buf);
    if(req != REQ_COMPLETE){
    }
    //
   
    buff_t res_buff;
    int result = get_resource(&res_buff);
    result = build_success_res(&res_buff);

    // this needs to loop (in case all data wasn't sent)
    send(conn_sock_fd,res_buff.data, strlen(res_buff.data), 0);
    // shutdown(conn_sock_fd, SHUT_RDWR); 

    printf("%s\n", res_buff.data);
    return EXIT_SUCCESS;
}
