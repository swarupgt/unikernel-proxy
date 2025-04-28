#include "proxy.h"
#include "auth_handler.h"
#include "utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>


LogContext log_ctx;

const char* test_name = "01_test_auth_handler";

client_t conn_bufs[MAX_CLIENTS];
int target_fds[MAX_TARGETS];

const int target_ports[TARGET_COUNT] = {
    9090, 9091, 9092, 9093, 9094,
    9095, 9096, 9097, 9098, 9099
};

int next_target = 0;
int target_client[MAX_TARGETS];

const char *http_unauthorized_response =
    "HTTP/1.1 401 Unauthorized\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 13\r\n"
    "Connection: close\r\n"
    "\r\n"
    "Unauthorized\n";


void test_unauthorized() {
    const char* ut_name = "test_unauthorized";
    setenv("AUTH_TOKEN", "token", 1);

    // case: Header `X-Auth-Token: <token>` not present
    char* headers[] = {"header1", NULL};
    assert(is_authorized(headers) == 0);

    // case: Header `X-Auth-Token: <token>` present, but invalid
    char* headers2[] = {"X-Auth-Token: invalid-token", NULL};
    assert(is_authorized(headers2) == 0);

    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", test_name, ut_name);
    unsetenv("AUTH_TOKEN");
}

void test_authorized() {
    const char* ut_name = "test_authorized";

    setenv("AUTH_TOKEN", "token", 1);
    // case: Header `X-Auth-Token: <token>` present and valid
    char* headers[] = {"X-Auth-Token: token", NULL};
    assert(is_authorized(headers) == 1);

    setenv("AUTH_TOKEN", "a.different.token", 1);
    // case: Header `X-Auth-Token: <token>` present and valid
    char* headers2[] = {"X-Auth-Token: a.different.token", NULL};
    assert(is_authorized(headers2) == 1);
    
    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", test_name, ut_name);
    unsetenv("AUTH_TOKEN");
}


int main() {
    int log_fd = get_file_logger(&log_ctx, "log_out.txt");
    test_unauthorized();
    test_authorized();
    close(log_fd);
}