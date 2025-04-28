#ifndef UNIKERNEL_PROXY_H
#define UNIKERNEL_PROXY_H

#include "utils/logger.h"

#define LISTEN_PORT 8080
#define FORWARD_PORT 9090
#define MAX_EVENTS 1000
#define MAX_CLIENTS 100
#define MAX_TARGETS 10
#define MAX_HEADERS 20
#define BUFFER_SIZE 4096
#define TOKEN_SIZE 10
#define TARGET_COUNT 10

// local host
#define FORWARD_HOST "127.0.0.1"

// qemu host
// #define FORWARD_HOST "10.0.2.2"

extern LogContext log_ctx;

typedef struct client_buffer {
    int fd;
    int read_len;
    int write_len;
    char read_buffer[BUFFER_SIZE];
    char write_buffer[BUFFER_SIZE];
} client_t;


extern client_t conn_bufs[MAX_CLIENTS];
extern int target_fds[MAX_TARGETS];
extern const int target_ports[TARGET_COUNT];
extern int next_target;
extern int target_client[MAX_TARGETS];
extern const char *http_unauthorized_response;

#endif