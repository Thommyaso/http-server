#ifndef __ROUTER__
#define __ROUTER__

#include "utils/buff.h"
#include "connection.h"
#include "parser.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define RESOURCE_READ_SUCCES 0

#define RESOURCE_READ_FAILURE 1

#define MAX_FILEPATH 500

#define FILEPATH_INCREMENT 100

typedef enum {
    RESPONSE_200 = 200,
    RESPONSE_404 = 404,
    RESPONSE_500 = 500
} res_code_t;

typedef struct {
    int t_size;
    char *data;
} resource_t;

res_code_t find_resource(res_buff_t *res_buff, headers_map_t *headers_map);

int combine_filepath(char *dest, char *src, int max_len);

int init_filepath(res_buff_t *res_buff, size_t len);

int append_filepath(res_buff_t *res_buff, char *str);

#endif
