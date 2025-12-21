#include "./router.h"
#include "../http/parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

int get_resource(req_buf_t req_buf, resource_t **presource){
    char filepath[100];
    sprintf(filepath, "%s/index.html", ROOT);

    struct stat st;
    stat(filepath, &st);
    size_t size = st.st_size;

    resource_t resource = {
        .t_size = size,
    };

    resource.data = malloc(size);

    if(resource.data == NULL){
        // oh no, what now? ram so expensive!
        return RESOURCE_READ_FAILURE;
    }

    FILE *fp = fopen(filepath, "rb");
    size_t result = fread(resource.data, 1, resource.t_size, fp);
    resource.data[result] = '\0';

    if(size == result){
        *presource = &resource;
        return RESOURCE_READ_SUCCES;
    }

    return RESOURCE_READ_FAILURE;
}
