#include "./network/listener.h"
#include "./network/connection.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    fd_t lis_fd = server_listen();
    if(lis_fd < 0){
        return EXIT_FAILURE;
    }

    server_run(lis_fd);

    return EXIT_SUCCESS;
}
