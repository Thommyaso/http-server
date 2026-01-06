#include <string.h>
#include "connection.h"
#include "parser.h"
#include "utils/buff.h"

int parse_req(buff_t *buff, headers_map_t *header_map)
{
    char *header_end = strstr(buff->data, HEADER_END);

    //header not complete tell connnection to keep waiting for rest of the info
    if(header_end == NULL) return REQ_NOT_COMPLETE;

    char *headers_start = parse_line_section(buff->data, header_map);

    if(!headers_start || headers_start - buff->data > buff->size_used) return REQ_FAIL;

    int result = parse_headers_section(buff, headers_start, header_map);

    return result;
}

char *parse_line_section(char *cursor_pos, headers_map_t *status_line)
{
    int idx;

    char *line_end = strstr(cursor_pos, LINE_END);
    if(!line_end) return NULL; // if line end not found, means header are malformed
    *line_end = '\0';

    for(idx = 0; cursor_pos < line_end && idx < 3; idx++){
        char *section_end = strchr(cursor_pos, STATUS_LINE_DELIMITER);

        if(!section_end) section_end = line_end; // last value in status line
        *section_end = '\0';
        
        switch (idx) {
            case 0:
                status_line->method = cursor_pos;
                break;
            case 1:
                status_line->url = cursor_pos;
                break;
            case 2:
                status_line->http = cursor_pos;
                break;
        }

        cursor_pos = section_end + 1;
    }

    if(idx != 3){
        // line section should have 3 elements, method, url and http version,
        // If it doesnt, it means headers are malformed
        // return unsuccessfull response
        return NULL;
    }

    //every line in http ends with "\r\n" so once i find it i move 2 postions right to find start of headers
    return line_end + 2;
}

int parse_headers_section(buff_t *buff, char *cursor_pos, headers_map_t *header_map)
{
    int line;
    char *line_end = NULL;
    for(line = 0; cursor_pos <  buff->data + buff->size_used && line < HEADER_LIMIT; line++) {
        line_end = strstr(cursor_pos, LINE_END);

        // something went wrong, every line is supposed to have "\r\n" ending, send failed response
        if(!line_end) return REQ_FAIL;

        *line_end = '\0';
        
        // headers ended (empty line encountered)
        if(strlen(cursor_pos) == 0) break; 
        
        char *label_end = strchr(cursor_pos, HEADER_DELIMITER);

        // every header is meant to have delimiter ':' if not send failed response
        if(!label_end) return REQ_FAIL;

        *label_end = '\0';
        
        header_map->headers[line].label = cursor_pos;

        cursor_pos = label_end + 1;
        while (*cursor_pos == ' ' || *cursor_pos == '\t') cursor_pos++;

        header_map->headers[line].value = cursor_pos;

        //every line in http ends with "\r\n" so once i find it i move 2 postions right to get next line
        cursor_pos = line_end + 2;
    }

    header_map->header_count = line + 1;

    return REQ_COMPLETE;
}

