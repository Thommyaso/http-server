#include "./response.h"
#include <stdio.h>
#include <string.h>
#include "../utils/buff.h"

int build_success_res(buff_t *resource)
{
    char header[200]; 
    int content_size = strlen(resource->data); // at this stage the only thing in respose buffer is the content, so das ist gut
    snprintf(header, sizeof header, RESPONSE_FMT, content_size);

    int result = buff_prepend(resource, header);
    return result;
}
