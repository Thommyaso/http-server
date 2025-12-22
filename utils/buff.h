#ifndef __BUFF_UTIL__
#define __BUFF_UTIL__

#include <string.h>

#define BUFF_SIZE 2000

typedef struct {
    size_t size;
    size_t used;
    char *data;
} buff_t;

int init_buff(buff_t *buff, size_t size);

void kill_buff(buff_t *buff);

int buff_prepend(buff_t *buff, char *prefix);

int buff_append(buff_t *buff, char *addon);

size_t calc_buff_size(buff_t *buff, size_t new_data);

int buff_increase(buff_t *buff, size_t size);

#endif
