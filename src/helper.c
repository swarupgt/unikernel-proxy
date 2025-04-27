#include "proxy.h"
#include "helper.h"

#include <string.h>
#include <stdlib.h>


char** get_http_headers(char* request) {

    char* request_copy = strdup(request);

    char** headers = malloc(sizeof(char*) * (MAX_HEADERS + 1));
    int header_count = 0;

    char* line = strtok(request_copy, "\r\n");
    line = strtok(NULL, "\r\n");

    while (line && header_count < MAX_HEADERS) {
        // stop at empty line
        if (strlen(line) == 0) break;

        headers[header_count++] = strdup(line);
        line = strtok(NULL, "\r\n");
    }

    headers[header_count] = NULL; // null terminate array
    free(request_copy);
    return headers;
}


void free_headers(char** headers) {
    for (int i = 0; i < MAX_HEADERS; i++) {
        free(headers[i]);
    }
    free(headers);
}


