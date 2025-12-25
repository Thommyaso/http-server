#include <asm-generic/socket.h>
#include <errno.h>
#include <sys/wait.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include "listener.h"

sock_fd_t server_listen()
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
        // printf("this is ip: %s, ip version: %s\n", ipstr, ipver);
    }


    sock_fd_t sock_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    
    // a way for me to make sure that socket connection doesn't fail when i restart the program
    int srv_reuse_opt = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &srv_reuse_opt, sizeof(srv_reuse_opt));
    //

    int bind_result = bind(sock_fd, servinfo->ai_addr, servinfo->ai_addrlen);
    int listen_result = listen(sock_fd, BACKLOG);
    Signal(SIGCHLD, reap_zombies);

    return sock_fd;

    freeaddrinfo(servinfo);
    return EXIT_SUCCESS;
}

void reap_zombies(int signo)
{
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0) 
        ;
    errno = saved_errno;
}

child_callback *Signal(int sigtype, child_callback *func)
{
    struct sigaction act, oact;

    act.sa_flags = 0;
    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
        
    if(sigaction(sigtype, &act, &oact) < 0){
        exit(SIG_ERR);
    };

    return oact.sa_handler;
}
