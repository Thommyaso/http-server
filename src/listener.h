#ifndef LISTENER_H
#define LISTENER_H

#define BACKLOG 25     // how many pending connections queue holds

typedef struct {
    int t_size;
    char *mem;
} t_buf;

typedef int fd_t; // just there to make it clear what the int is

typedef void child_callback(int pid);

fd_t server_listen();

void reap_zombies(int pid);

child_callback *Signal(int sigtype, child_callback *func);

#endif
