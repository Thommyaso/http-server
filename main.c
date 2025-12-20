#include "./network/listener.h"
#include "./network/connection.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    sock_fd_t lis_sock_fd = server_listen();
    server_run(lis_sock_fd);

    return EXIT_SUCCESS;
}
