#include "proxy.h"
#include "helper.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char* test_name = "03_test_helper";

LogContext log_ctx;

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


void test_get_http_headers() {
    const char* ut_name = "test_get_http_headers";
    
    char request[] = 
        "GET / HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "X-Auth-Token: token\r\n"
        "\r\n";
    
    // call helper
    char** headers = get_http_headers(request);
    // expect 2 headers then a NULL
    assert(headers[0] && strcmp(headers[0], "Host: example.com") == 0);
    assert(headers[1] && strcmp(headers[1], "X-Auth-Token: token") == 0);
    assert(headers[2] == NULL);

    free_headers(headers);
    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", 
    test_name, ut_name);
}

void test_free_headers() {
    const char* ut_name = "test_free_headers";

    char request[] = 
        "GET /test HTTP/1.1\r\n"
        "X-Test: value\r\n"
        "\r\n";
    
    char** headers = get_http_headers(request);
    assert(headers != NULL);
    assert(headers[0] && strcmp(headers[0], "X-Test: value") == 0);
    free_headers(headers);

    char** empty = malloc(sizeof(char*) * (MAX_HEADERS + 1));
    for(int i = 0; i <= MAX_HEADERS; i++) empty[i] = NULL;
    free_headers(empty);

    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", 
    test_name, ut_name);
}

int main() {
    test_get_http_headers();
    test_free_headers();
    return 0;
}