#include "./response.h"
#include <stdio.h>
#include <string.h>
#include "../utils/buff.h"

int build_success_res(buff_t *res_buff)
{
    char header[200]; 

    // at this stage the only thing in respose buffer is the content, so das ist gut
    int content_size = strlen(res_buff->data);
    snprintf(header, sizeof header, RESPONSE_FMT, content_size);

    int result = buff_prepend(res_buff, header);
    return result;
}
