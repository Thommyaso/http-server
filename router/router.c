#include "./router.h"
#include "../http/parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "../utils/buff.h"

int get_resource(buff_t *buff)
{
    int result;
    char filepath[100];
    sprintf(filepath, "%s/index.html", ROOT);

    struct stat st;
    stat(filepath, &st);
    size_t filesize = st.st_size;

    result =  buff_increase(buff, filesize);

    FILE *fp = fopen(filepath, "rb");
    size_t fread_result = fread(buff->data + buff->used, 1, buff->size, fp);
    buff->data[buff->used + fread_result] = '\0';
    buff->used += fread_result;

    if(filesize == fread_result){
        return 0;
    }

    return 1;
}
