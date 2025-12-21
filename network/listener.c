#include <asm-generic/socket.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BACKLOG 25     // how many pending connections queue holds

typedef struct {
    int t_size;
    char *mem;
} t_buf;

typedef int lis_sock_fd;

// AF stands for address family
// PF stands for protocol family

lis_sock_fd server_listen()
{
    struct addrinfo hints;
    struct addrinfo *servinfo;
    char ipstr[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 

    int status;
    if((status = getaddrinfo(NULL, "8080", &hints, &servinfo)) != 0){
        printf("gai error: %s\n", gai_strerror(status));
        return EXIT_FAILURE;
    }

    struct addrinfo *p;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;
        struct sockaddr_in *ipv4;
        struct sockaddr_in6 *ipv6;

        if(p->ai_family == AF_INET){
            ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else {
            ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        inet_ntop(p->ai_family,addr, ipstr, sizeof ipstr);
        printf("this is ip: %s, ip version: %s\n", ipstr, ipver);
    }

    lis_sock_fd sock_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    int srv_reuse_opt = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &srv_reuse_opt, sizeof(srv_reuse_opt));

    int bind_result = bind(sock_fd, servinfo->ai_addr, servinfo->ai_addrlen);
    int listen_result = listen(sock_fd, BACKLOG);

    return sock_fd;

    freeaddrinfo(servinfo);
    return EXIT_SUCCESS;
}

