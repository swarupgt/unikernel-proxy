#include "proxy.h"
#include "conn_handler.h"
#include "utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

LogContext log_ctx;

const char* test_name = "02_test_conn_handler";

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


void reset_values(){
    memset(conn_bufs, 0, sizeof(conn_bufs));
    memset(target_fds, 0, sizeof(target_fds));
    memset(target_client, 0, sizeof(target_client));
}


void test_init_conn_buf() {
    const char* ut_name = "test_init_conn_buf";
    
    reset_values();
    init_conn_buf();
    for(int i=0; i< MAX_CLIENTS; i++) {
        assert(conn_bufs[i].fd == -1);
    }

    printf("test:%s:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n", test_name, ut_name);
}



int main() {
    get_file_logger(&log_ctx, "log_out.txt");
    
    test_init_conn_buf();

}