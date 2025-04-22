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

#define LISTEN_PORT 8080
#define FORWARD_PORT 9090
#define MAX_EVENTS 1000
#define MAX_CLIENTS 100
#define MAX_TARGETS 1
#define MAX_HEADERS 20
#define BUFFER_SIZE 4096
#define TOKEN_SIZE 10

// local host
// #define FORWARD_HOST "127.0.0.1"

// qemu host
#define FORWARD_HOST "10.0.2.2"
#define AUTH_HEADER "X-Auth-Token"
#define VALID_TOKEN "token"

typedef struct client_buffer {
    int fd;
    int read_len;
    int write_len;
    char read_buffer[BUFFER_SIZE];
    char write_buffer[BUFFER_SIZE];
} client_t;

client_t conn_bufs[MAX_CLIENTS];
int target_fds[MAX_TARGETS];

const char *http_unauthorized_response =
    "HTTP/1.1 401 Unauthorized\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 13\r\n"
    "Connection: close\r\n"
    "\r\n"
    "Unauthorized\n";

// fn decs

int make_socket_non_blocking(int sfd);
int create_server_socket(int port);
void connect_to_targets();
void init_conn_buf();

int forward_data(int epfd, int conn_idx, int target_fd, int len);
char** get_http_headers(char* request);
void free_headers(char** headers);
int is_authorized(char** headers);

int is_target_fd(int fd);
int is_client_fd(int fd);
int get_optimal_target_fd();

int get_lowest_conn_buf();
int get_conn_buf_from_fd(int fd);
void reset_conn_buff(int client_idx);


// MAIN

int main() {

    init_conn_buf();

    int epfd = epoll_create1(0);
    if (epfd == -1) {
        printf("cannot create epoll\n");
        exit(EXIT_FAILURE);
    }

    int listen_fd = create_server_socket(LISTEN_PORT);
    struct epoll_event ev, events[MAX_EVENTS];

    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    printf("listening on port %d...\n", LISTEN_PORT);

    while (1) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; ++i) {

            int fd = events[i].data.fd;

            // new conns
            if (fd == listen_fd) {
                struct sockaddr_in client_addr;
                socklen_t addr_len = sizeof(client_addr);
                int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addr_len);
                if (client_fd < 0) continue;

                make_socket_non_blocking(client_fd);
                
                ev.events = EPOLLIN;
                ev.data.fd = client_fd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
                printf("new connection accepted\n");

                int conn_idx = get_lowest_conn_buf();
                if (conn_idx < 0) {
                    // too many active clients, close fd?
                    printf("too many active clients, closing fd for now\n");
                    epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, NULL);
                    close(fd);
                    continue;
                }

                printf("new client fd: %d\n", client_fd);
                conn_bufs[conn_idx].fd = client_fd;

            // read client req
            } else if (is_client_fd(fd) && events[i].events == EPOLLIN) {

                int conn_idx = get_conn_buf_from_fd(fd);
                int len = read(fd, conn_bufs[conn_idx].read_buffer, sizeof(conn_bufs[conn_idx].read_buffer));
                if (len < 0) {
                    printf("read from client failed\n");
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                    continue;
                }

                // http parsing
                // assume they send the correct headers for now
                char **headers = get_http_headers(conn_bufs[conn_idx].read_buffer);

                // auth
                if (!is_authorized(headers)) {
                    // buffer to write is unauthorized string, 
                    // set that in conn_bufs of that client
                    // and make client fd epollout

                    printf("client fd: %d is not authorized\n", fd);
                    strcpy(conn_bufs[conn_idx].write_buffer, http_unauthorized_response);
                    events[i].events = EPOLLOUT;
                    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &events[i]);
                    conn_bufs[conn_idx].write_len = strlen(http_unauthorized_response);
                    continue;
                }

                free_headers(headers);

                printf("client fd: %d is authorized\n", fd);

                // choose target to forward data to based on some load balancing logic
                int target_fd = get_optimal_target_fd();

                // forward data and wait for response
                // TODO: add target_fd response also to epoll for better scaling and cleaner code
                if (forward_data(epfd, conn_idx, target_fd, len) < 0) {
                    printf("forwarding data to target fd: %d failed, service down\n", target_fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, target_fd, NULL);
                    close(target_fd);
                    exit(EXIT_FAILURE);
                }

                // get response, modify client fd to epollout to write 
                len = read(target_fd, conn_bufs[conn_idx].write_buffer, sizeof(conn_bufs[conn_idx].write_buffer));
                if (len < 0) {
                    printf("could not read from target fd: %d\n", target_fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, target_fd, NULL);
                    close(target_fd);
                    exit(EXIT_FAILURE);
                }

                conn_bufs[conn_idx].write_len = len;
                events[i].events = EPOLLOUT;
                epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &events[i]);
                close(target_fd);
                continue;
            
            // respond to client req
            } else if (is_client_fd(fd) && events[i].events == EPOLLOUT) {

                int idx = get_conn_buf_from_fd(fd);
                if (write(fd, conn_bufs[idx].write_buffer, conn_bufs[idx].write_len) < 0) {
                    printf("could not write to client fd: %d\n", fd);
                } else {
                    printf("success responding to client fd:%d\n", fd);
                }

                epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                close(fd);              
                // done serving client, reset conn buff for any new clients
                reset_conn_buff(idx);
            }

            // TODO:
            // handle target fds as part of epoll (if we make targets non-blocking)
        }
    }

    close(epfd);
    return 0;
}

// fn defs, structure later

int make_socket_non_blocking(int sfd) {
    int flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(sfd, F_SETFL, flags | O_NONBLOCK);
}

int create_server_socket(int port) {
    int sockfd;
    struct sockaddr_in addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket failed\n");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("bind failed\n");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 10) < 0) {
        printf("listen failed\n");
        exit(EXIT_FAILURE);
    }

    make_socket_non_blocking(sockfd);
    return sockfd;
}

// for now target_fds[] only has 1 target
void connect_to_targets() {
    int sockfd;
    struct sockaddr_in addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("cannot create target socket\n");
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(FORWARD_PORT);
    inet_pton(AF_INET, FORWARD_HOST, &addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("connect error\n");
        close(sockfd);
        return -1;
    }

    // make_socket_non_blocking(sockfd);
    target_fds[0] = sockfd;
}

int forward_data(int epfd, int conn_idx, int target_fd, int len) {

    if (write(target_fd, conn_bufs[conn_idx].read_buffer, len) < 0) {
        printf("write to service failed\n");
        return -1;
    }

    return 0;
}


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

// placeholder auth
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

// load balancing logic goes here
int get_optimal_target_fd() {

    // for now, just connecting and returning new conn to target service.
    // choosing 1 service.
    connect_to_targets();
    return target_fds[0];
}

// initialise client connections buffer array
void init_conn_buf() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        conn_bufs[i].fd = -1;
        memset(conn_bufs[i].write_buffer, 0, sizeof(conn_bufs[i].write_buffer));
        memset(conn_bufs[i].read_buffer, 0, sizeof(conn_bufs[i].read_buffer));
        conn_bufs[i].read_len = 0;
        conn_bufs[i].write_len = 0;
    }
}

// get lowest available conn buff
int get_lowest_conn_buf() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (conn_bufs[i].fd == -1) {
            return i;
        } else {
            printf("fd is %d\n", conn_bufs[i].fd);
        }
    }
    return -1;
}

// get the connection buf from fd
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

void free_headers(char** headers) {
    for (int i = 0; i < MAX_HEADERS; i++) {
        free(headers[i]);
    }
    free(headers);
}