#include "proxy.h"
#include "auth_handler.h"
#include "utils/logger.h"
#include <string.h>
#include <stdlib.h>

static char* get_valid_token() {
    char* token = getenv("AUTH_TOKEN");
    if( !token ){
        log_error(&log_ctx, "fatal: env variable AUTH_TOKEN not set\n");
        exit(1);
    }
    return token;
}

int is_authorized(char** headers) {
    for (int i = 0; headers[i]; i++) {
        const char* header = headers[i];
        if (strncmp(header, AUTH_HEADER, 12) == 0) {

            const char* value = header + 13;
            while (*value == ' ' || *value == '\t') value++;

            if (strcmp(value, get_valid_token()) == 0) {
                return 1; // Match found
            }
        }
    }
    log_warn(&log_ctx, "unauthorized request\n");
    return 0;
}