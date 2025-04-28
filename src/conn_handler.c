#include "proxy.h"
#include "conn_handler.h"
#include "utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>


void init_conn_buf() {
    log_info(&log_ctx, "initializing connection buffers...\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        conn_bufs[i].fd = -1;
        memset(conn_bufs[i].write_buffer, 0, sizeof(conn_bufs[i].write_buffer));
        memset(conn_bufs[i].read_buffer, 0, sizeof(conn_bufs[i].read_buffer));
        conn_bufs[i].read_len = 0;
        conn_bufs[i].write_len = 0;
    }
    log_info(&log_ctx, "initialized connection buffers\n");
}

__attribute__((weak))
int connect_to_targets(int port) {
    int sockfd;
    struct sockaddr_in addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        log_error(&log_ctx, "cannot create target socket\n");
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, FORWARD_HOST, &addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        log_error(&log_ctx, "connect error\n");
        close(sockfd);
        return -1;
    }

    make_socket_non_blocking(sockfd);
    return sockfd;
}


int make_socket_non_blocking(int sfd) {
    int flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(sfd, F_SETFL, flags | O_NONBLOCK);
}


int create_server_socket(int port) {
    int sockfd;
    struct sockaddr_in addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        log_error(&log_ctx, "socket failed\n");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        log_error(&log_ctx, "bind failed\n");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 10) < 0) {
        log_error(&log_ctx, "listen failed\n");
        exit(EXIT_FAILURE);
    }

    make_socket_non_blocking(sockfd);
    return sockfd;
}


int forward_data(int conn_idx, int target_fd, int len) {

    if (write(target_fd, conn_bufs[conn_idx].read_buffer, len) < 0) {
        log_error(&log_ctx, "write to service failed\n");
        return -1;
    }

    return 0;
}


int is_target_fd(int fd) {
    for (int i = 0; i < MAX_TARGETS; i++) {
        if (fd == target_fds[i]) {
            return 1;
        }
    }
    return 0;
}

int is_client_fd(int fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (fd == conn_bufs[i].fd) {
            return 1;
        }
    }
    return 0;
}


int get_lowest_conn_buf() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (conn_bufs[i].fd == -1) {
            return i;
        }    
    }
    return -1;
}

int get_conn_buf_from_fd(int fd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (fd == conn_bufs[i].fd) {
            return i;
        }
    }
    return -1;
}

void reset_conn_buff(int client_idx) {
    conn_bufs[client_idx].fd = -1;
    memset(conn_bufs[client_idx].write_buffer, 0, sizeof(conn_bufs[client_idx].write_buffer));
    memset(conn_bufs[client_idx].read_buffer, 0, sizeof(conn_bufs[client_idx].read_buffer));
    conn_bufs[client_idx].read_len = 0;
    conn_bufs[client_idx].write_len = 0;
}