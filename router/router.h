#ifndef __ROUTER__
#define __ROUTER__

#include "../utils/buff.h"
#include "../network/connection.h"
#include "../http/parser.h"

#define ROOT "/home/thomas/Repositories/www/lillicrap/public"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define RESOURCE_READ_SUCCES 0

#define RESOURCE_READ_FAILURE 1

#define MAX_FILEPATH 500

typedef enum {
    RESPONSE_200 = 200,
    RESPONSE_404 = 404,
    RESPONSE_500 = 500
} res_code_t;

typedef struct {
    int t_size;
    char *data;
} resource_t;

res_code_t get_resource(buff_t *buff, headers_map_t *headers_map);

void remove_trailing_slash(char *filepath);

int combine_filepath(char *dest, char *src, int max_len);

#endif
