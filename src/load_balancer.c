#include "proxy.h"
#include "helper.h"
#include "load_balancer.h"
#include "conn_handler.h"
#include <stdio.h>

int get_optimal_target_fd(int conn_idx) {
    int idx = next_target;
    int port = target_ports[idx];
    int sockfd = connect_to_targets(port);
    if (sockfd >= 0) {
        target_fds[idx]   = sockfd;
        target_client[idx] = conn_idx;
    }
    next_target = (next_target + 1) % TARGET_COUNT;
    return sockfd;
}
