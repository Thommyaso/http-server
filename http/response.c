#include "../router/router.h"
#include "./response.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int build_success_res(resource_t *resource)
{
    char header[200]; 
    snprintf(header, sizeof header, RESPONSE_FMT, resource->t_size);

    size_t len_p = strlen(header);
    resource->data = prepend(resource->data, header);
    resource->t_size += len_p;

    return 0;
}

char *prepend(char *data, char *prefix){
    size_t len_s = strlen(data);
    size_t len_p = strlen(prefix);
    size_t len_total = len_s + len_p + 1;

    data = realloc(data, len_total);
    memmove(data + len_p, data, len_s + 1);
    memcpy(data, prefix, len_p);

    return data;
}
