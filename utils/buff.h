#ifndef __BUFF_UTIL__
#define __BUFF_UTIL__

#include <fcntl.h>
#include <string.h>
#include "../network/listener.h"

#define BUFF_SIZE 2048

typedef struct {
    size_t total_size;
    size_t size_used;
    size_t size_processed;
    char *data;
} buff_t;

typedef struct {
    buff_t base;
    char *filepath;
    size_t filesize;
    size_t filepath_total_len;
    off_t size_uploaded;
    fd_t file_fd;
} res_buff_t;

int init_buff(buff_t *buff, size_t size);

int init_res_buff(res_buff_t *res_buff, size_t new_data);

void kill_buff(buff_t *buff);

void kill_res_buff(res_buff_t *res_buff);

int buff_prepend(buff_t *buff, char *prefix);

int buff_append(buff_t *buff, char *addon);

size_t calc_buff_size(buff_t *buff, size_t new_data);

int buff_increase(buff_t *buff, size_t size);

#endif
