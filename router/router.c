#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "router.h"
#include "../http/parser.h"
#include "../utils/buff.h"

#define FILEPATH_INCREMENT 100

/*
 * Simple resource retrival, if url is a directory it looks for index.html in that dir,
 * otherwise it looks for the file reqested
 * updates buffer -- at this point buffer is only meant to contain this filecontent so content-length
 * can be derived from used buffer size
 *
 * Returns resonse code
 */
res_code_t find_resource(res_buff_t *res_buff, headers_map_t *headers_map)
{
    int fail;

    res_buff->size_uploaded = 0;
    res_buff->filesize = 0;
    if(res_buff->filepath == NULL){
        fail = init_filepath(res_buff, FILEPATH_INCREMENT);
        if(fail) return RESPONSE_500;
    }
    res_buff->filepath[0] = '\0';

    fail = append_filepath(res_buff, ROOT_DIR);
    if(fail) return RESPONSE_500;
    fail = append_filepath(res_buff, headers_map->url);
    if(fail) return RESPONSE_500;

    struct stat st;
    if(stat(res_buff->filepath, &st) != 0) return RESPONSE_500;

    if (S_ISDIR(st.st_mode)) {
        remove_trailing_slash(res_buff->filepath);

        fail = append_filepath(res_buff, "/index.html");
        if(fail) return RESPONSE_500;

        if(stat(res_buff->filepath, &st) != 0) return RESPONSE_404; // file not found

    } else if (!S_ISREG(st.st_mode)) {
        // not a regular file or directory = bad request
        return RESPONSE_404;
    }

    res_buff->filesize = st.st_size;
    res_buff->file_fd = open(res_buff->filepath, O_RDONLY);
    if(res_buff->file_fd < 0) return RESPONSE_500;

    return RESPONSE_200;
}

int init_filepath(res_buff_t *res_buff, size_t len)
{
    res_buff->filepath = malloc(len);
    if(res_buff->filepath == NULL) {
        res_buff->filepath_total_len = 0;
        return 1;
    }

    res_buff->filepath[0] = '\0';
    res_buff->filepath_total_len = len;
    return 0;
}

int append_filepath(res_buff_t *res_buff, char *str)
{
    int str_len = strlen(str);

    if(res_buff->filepath_total_len - strlen(res_buff->filepath) <= str_len){
        res_buff->filepath_total_len += FILEPATH_INCREMENT > str_len ? FILEPATH_INCREMENT : str_len + 1;
        res_buff->filepath = realloc(res_buff->filepath, res_buff->filepath_total_len);

        if(res_buff->filepath == NULL){
            res_buff->filepath_total_len = 0;
            return 1;
        }
    }

    strcat(res_buff->filepath, str);
    return 0;
}

/*
* Helper function to remove trailing forward slash if it exists
* If it doesn't exist nothing happens
* This ensures we can safely append file to the directory path
*/
void remove_trailing_slash(char *filepath)
{
    int end_idx = strlen(filepath) - 1;
    if(filepath[end_idx] != '/') return;
    filepath[end_idx] = '\0';
}
