#ifndef ROUTER_H
#define ROUTER_H

#include "../network/connection.h"

#define ROOT "/home/thomas/Repositories/hw"
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define RESOURCE_READ_SUCCES 0
#define RESOURCE_READ_FAILURE 1

typedef struct {
    int t_size;
    char *data;
} resource_t;

int get_resource(req_buf_t req_buf, resource_t **resource);
#endif
