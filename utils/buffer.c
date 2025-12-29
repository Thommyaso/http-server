#include <stdlib.h>
#include <string.h>
#include "./buff.h"
#include <sys/param.h>

int init_buff(buff_t *buff, size_t new_data)
{
    size_t size = calc_buff_size(NULL, new_data);

    buff->data =  malloc(size);
    if(buff->data == NULL){
        //oh no, ram so expensive!
        return 1;
    }
    buff->total_size = size;
    buff->size_used = 0;
    buff->size_processed = 0;

    return 0;
}

int init_res_buff(res_buff_t *res_buff, size_t new_data)
{
    int fail = init_buff(&res_buff->base, 0);
    if(fail) return 1;
    
    res_buff->filepath = NULL;
    res_buff->size_uploaded = 0;
    return 0;
}

void kill_buff(buff_t *buff)
{
    if(buff->data == NULL) return;
    free(buff->data);
    buff->data = NULL;
}

void kill_res_buff(res_buff_t *res_buff)
{
    kill_buff(&res_buff->base);

    if(res_buff->filepath == NULL) return;
    free(res_buff->filepath);
    res_buff->filepath = NULL;
}

int buff_prepend(buff_t *buff, char *prefix)
{
    size_t len_prefix = strlen(prefix);
    size_t new_buff_size = calc_buff_size(buff, len_prefix);

    if(new_buff_size){
        buff->data = realloc(buff->data, new_buff_size);
        if(buff->data == NULL){
            return 1;
        }
        buff->total_size = new_buff_size;
    }

    memmove(buff->data + len_prefix, buff->data, buff->size_used);
    memcpy(buff->data, prefix, len_prefix);
    buff->size_used += len_prefix;
    buff->data[buff->size_used] = '\0';

    return 0;
}

int buff_append(buff_t *buff, char *addon)
{
    size_t len_addon = strlen(addon);
    size_t new_buff_size = calc_buff_size(buff, len_addon);

    if(new_buff_size){
        buff->data = realloc(buff->data, new_buff_size);
        if(buff->data == NULL){
            return 1;
        }
        buff->total_size = new_buff_size;
    }

    memcpy(buff->data + buff->size_used, addon, len_addon);
    buff->size_used += len_addon;
    buff->data[buff->size_used] = '\0';

    return 0;
}

int buff_increase(buff_t *buff, size_t size)
{
    size_t new_buff_size = calc_buff_size(buff, size);

    if(new_buff_size){
        buff->data = realloc(buff->data, new_buff_size);
        if(buff->data == NULL){
            return 1;
        }
        buff->total_size = new_buff_size;
    }

    return 0;
}

size_t calc_buff_size(buff_t *buff, size_t new_data)
{
    // int truncates values, this creates an offset,
    // so that anything over buff_size and under 2*buff_size will make multiplier 2 -- basically this works like celi();
    int multiplier = (BUFF_SIZE + MAX(1, new_data) - 1) / BUFF_SIZE;

    if(buff == NULL){
        return BUFF_SIZE * multiplier;
    }else if((buff->total_size - buff->size_used) > new_data){
        // this function can be called just in case, before populating data, so in case 
        // size doesn't need changing returns 0
        return 0;
    } else {
        return buff->total_size + BUFF_SIZE * multiplier;
    }
}
