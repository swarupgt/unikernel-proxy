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
#define MAX_EVENTS 10
#define BUFFER_SIZE 4096
#define FORWARD_HOST "10.0.2.2"

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

int connect_to_target(const char *host, int port) {
    int sockfd;
    struct sockaddr_in addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("cannot create target socket\n");
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("connect\n");
        close(sockfd);
        return -1;
    }

    make_socket_non_blocking(sockfd);
    return sockfd;
}

int forward_data(int epfd, int from, int to) {
    char buffer[BUFFER_SIZE];
    int len = read(from, buffer, sizeof(buffer));
    if (len < 0) {
        printf("read from client failed\n");
        epoll_ctl(epfd, EPOLL_CTL_DEL, from, NULL);
        close(from);
        return 0;
    }
    if (write(to, buffer, len) < 0 ) {
        printf("write to service failed\n");
        return -1;
    }

    return 0;
}

int main() {
    int epfd = epoll_create1(0);
    if (epfd == -1) {
        printf("cannot creat epoll\n");
        exit(EXIT_FAILURE);
    }

    int listen_fd = create_server_socket(LISTEN_PORT);
    struct epoll_event ev, events[MAX_EVENTS];

    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    int target_fd = connect_to_target(FORWARD_HOST, FORWARD_PORT);
    if (target_fd < 0) {
        printf("can't connect to target :(\n");
        close(epfd);
        exit(EXIT_FAILURE);
    }

    printf("listening on port %d, forwarding to port %d\n", LISTEN_PORT, FORWARD_PORT);

    while (1) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; ++i) {

            int fd = events[i].data.fd;

            if (fd == listen_fd) {
                struct sockaddr_in client_addr;
                socklen_t addr_len = sizeof(client_addr);
                int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addr_len);
                if (client_fd < 0) continue;

                make_socket_non_blocking(client_fd);
                
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_fd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
                printf("new connection accepted\n");

            } else if (fd != target_fd) {
                if (forward_data(epfd, fd, target_fd) < 0) {
                    printf("forwarding data failed, service down\n");
                    epoll_ctl(epfd, EPOLL_CTL_DEL, target_fd, NULL);
                    close(target_fd);
                    exit(EXIT_FAILURE);
                }
            } 
        }
    }

    close(epfd);
    return 0;
}
