#include <stdlib.h>
#include <string.h>
#include "./buff.h"

int init_buff(buff_t *buff, size_t size)
{
    // I want my buffer to re-size memory in buff_size steps only,
    // so i either apply BUFF_SIZE or if even the BUFF_SIZE is not big enough i multiply it till it is;
    if(size == 0){
        size = BUFF_SIZE;
    }else if(BUFF_SIZE < size){
        // int truncates values, this creates an offset,
        // so that anything over buff_size and under 2*buff_size will make multiplier 2 -- basically this works like celi();
        int multiplier = (BUFF_SIZE + size - 1) / size;

        size = BUFF_SIZE * multiplier;
    } else {
        size = BUFF_SIZE;
    }

    buff->data =  malloc(size);
    if(buff->data == NULL){
        //oh no, ram so expensive!
        return 1;
    }
    buff->size = size;

    return 0;
}

void kill_buff(buff_t *buff){
    free(buff->data);
}

int buff_prepend(buff_t *buff, char *prefix)
{
    size_t len_data = strlen(buff->data);
    size_t len_prefix = strlen(prefix);
    size_t free_space = buff->size - len_data;

    if(free_space < len_prefix){
        size_t new_size = buff->size * 2;

        buff->data = realloc(buff->data, new_size);
        if(buff->data == NULL){
            return 1;
        }
        buff->size = new_size;
    }

    memmove(buff->data + len_prefix, buff->data, len_data + 1);
    memcpy(buff->data, prefix, len_prefix);

    return 0;
}

int buff_append(buff_t *buff, char *addon)
{
    size_t len_data = strlen(buff->data);
    size_t len_addon = strlen(addon);
    size_t free_space = buff->size - len_data;

    if(free_space < len_addon){
        size_t new_size = buff->size * 2;

        buff->data = realloc(buff->data, new_size);
        if(buff->data == NULL){
            return 1;
        }
        buff->size = new_size;
    }

    memcpy(buff->data + len_data, addon, len_addon);

    return 0;
}
