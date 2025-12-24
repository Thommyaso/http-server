#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "router.h"
#include "../http/parser.h"
#include "../utils/buff.h"

/*
 * Simple resource retrival, if url is a directory it looks for index.html in that dir,
 * otherwise it looks for the file reqested
 * updates buffer -- at this point buffer is only meant to contain this filecontent so content-length
 * can be derived from used buffer size
 *
 * Returns resonse code
 */
res_code_t get_resource(buff_t *buff, headers_map_t *headers_map)
{
    char filepath[MAX_FILEPATH] = ROOT_DIR;

    int failed = combine_filepath(filepath, headers_map->url, MAX_FILEPATH);
    if(failed) return RESPONSE_500;

    struct stat st;
    if(stat(filepath, &st) != 0) return RESPONSE_500;

    if (S_ISDIR(st.st_mode)) {
        remove_trailing_slash(filepath);

        failed = combine_filepath(filepath, "/index.html", MAX_FILEPATH);
        if(failed) return RESPONSE_500;

        if(stat(filepath, &st) != 0) return RESPONSE_500;

    } else if (!S_ISREG(st.st_mode)) {
        // not a regular file or directory = bad request
        return RESPONSE_404;
    }

    size_t filesize = st.st_size;
    failed =  buff_increase(buff, filesize);
    if(failed) return RESPONSE_500;

    FILE *fp = fopen(filepath, "rb");
    size_t fread_result = fread(buff->data + buff->used, 1, buff->size, fp);
    buff->data[buff->used + fread_result] = '\0';
    buff->used += fread_result;

    if(filesize == fread_result){
        return RESPONSE_200;
    }

    return RESPONSE_500;
}

/*
 * This is a sanity check -- I didn't want to mess around setting runtime memory for filepath
 * Filepath has to be the combination of ROOT_DIR and whatever was received in http request
 * I have allocated max_len size for filepath.
 * It is unlikely but buffer could overflow
 * This function checks before combining filepaths together
 */
int combine_filepath(char *dest, char *src, int max_len)
{
    if(strlen(dest) + strlen(src) >= max_len) return 1;
    strcat(dest, src);
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
