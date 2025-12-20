#include <stdlib.h>
#include <sys/socket.h>

#include "./listener.h"
#include "./connection.h"
#include "../http//parser.h"

int server_run(sock_fd_t lis_sock_fd)
{
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    sock_fd_t conn_sock_fd = accept(lis_sock_fd, (struct sockaddr *)&client_addr, &client_addr_size);

    req_buf_t req_buf = {
        .capacity = 2000,
    };
    req_buf.data = malloc(req_buf.capacity * sizeof(*req_buf.data));

    if(req_buf.data == NULL){
        return EXIT_FAILURE;
    }

    int recv_size = recv(conn_sock_fd, req_buf.data, req_buf.capacity, 0);
    req_buf.used = recv_size;

    int res = parse_buf(&req_buf);

    return EXIT_SUCCESS;
}
