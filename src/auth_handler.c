#include "proxy.h"
#include <string.h>

int is_authorized(char** headers) {
    for (int i = 0; headers[i]; i++) {
        const char* header = headers[i];
        if (strncmp(header, AUTH_HEADER, 12) == 0) {

            const char* value = header + 13;
            while (*value == ' ' || *value == '\t') value++;
            if (strcmp(value, VALID_TOKEN) == 0) {
                return 1; // Match found
            }
        }
    }
    return 0;
}