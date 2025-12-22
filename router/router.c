#include "./router.h"
#include "../http/parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "../utils/buff.h"

int get_resource(buff_t *buff)
{
    char filepath[100];
    sprintf(filepath, "%s/index.html", ROOT);

    struct stat st;
    stat(filepath, &st);
    size_t filesize = st.st_size;

    int buf_result = init_buff(buff, filesize);
    if(buf_result != 0){
        // oh no malloc failed!
        return buf_result;
    }

    FILE *fp = fopen(filepath, "rb");
    size_t fread_result = fread(buff->data , 1, buff->size, fp);
    buff->data[fread_result] = '\0';

    if(filesize == fread_result){
        return 0;
    }

    return 1;
}
