#ifndef BUFF_UTIL
#define BUFF_UTIL = 1;

#include <string.h>

#define BUFF_SIZE 2000

typedef struct {
    int size;
    char *data;
} buff_t;

int init_buff(buff_t *buff, size_t size);
void kill_buff(buff_t *buff);
int buff_prepend(buff_t *buff, char *prefix);
int buff_append(buff_t *buff, char *addon);
#endif
