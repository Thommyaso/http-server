#ifndef LISTENER_H
#define LISTENER_H

#define BACKLOG 25     // how many pending connections queue holds

typedef struct {
    int t_size;
    char *mem;
} t_buf;

typedef int sock_fd_t; // just there to make it clear what the int is

sock_fd_t server_listen();

#endif
