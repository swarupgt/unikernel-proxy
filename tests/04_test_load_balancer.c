#include "proxy.h"
#include "load_balancer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>


int connect_to_targets(int port) {
    return port + 1000;
}


LogContext log_ctx;
client_t conn_bufs[MAX_CLIENTS];
int target_fds[MAX_TARGETS];
int target_client[MAX_TARGETS];
int next_target = 0;
const int target_ports[TARGET_COUNT] = {
    9090, 9091, 9092, 9093, 9094,
    9095, 9096, 9097, 9098, 9099
};


void reset_globals() {
    memset(target_fds,   0, sizeof(target_fds));
    memset(target_client, 0, sizeof(target_client));
    next_target = 0;
}

void test_round_robin_allocation() {
    const char* ut_name = "test_round_robin_allocation";
    reset_globals();

    int fd0 = get_optimal_target_fd(5);
    assert(fd0 == target_ports[0] + 1000);
    assert(target_fds[0] == fd0);
    assert(target_client[0] == 5);

    int fd1 = get_optimal_target_fd(6);
    assert(fd1 == target_ports[1] + 1000);
    assert(target_fds[1] == fd1);
    assert(target_client[1] == 6);

    for (int i = 2; i < TARGET_COUNT; i++) {
        int cidx = i + 10;
        int fdi = get_optimal_target_fd(cidx);
        assert(fdi == target_ports[i] + 1000);
        assert(target_fds[i] == fdi);
        assert(target_client[i] == cidx);
    }

    // wrap‑around
    int fd_wrap = get_optimal_target_fd(99);
    assert(fd_wrap == target_ports[0] + 1000);
    assert(target_fds[0] == fd_wrap);
    assert(target_client[0] == 99);

    printf("test:04_test_load_balancer:\033[1;33m%s\033[0m:\033[32mPASSED\033[0m\n",
           ut_name);
}

int main() {
    test_round_robin_allocation();
    return 0;
}