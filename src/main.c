#include "proxy.h"
#include "helper.h"
#include "auth_handler.h"
#include "load_balancer.h"
#include "conn_handler.h"

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
        // printf("num epoll-events: %d\n", n);
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
                printf("assigned conn index: %d\n", conn_idx);
                if (conn_idx < 0) {
                    // too many active clients, close fd?
                    printf("too many active clients, closing fd for now\n");
                    epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, NULL);
                    close(fd);
                    continue;
                }

                printf("new client fd: %d\n", client_fd);
                conn_bufs[conn_idx].fd = client_fd;
                continue;

            // read client req
            } else if (is_client_fd(fd) && (events[i].events & EPOLLIN) ) {
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
                    ev.data.fd = fd;
                    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &events[i]);
                    conn_bufs[conn_idx].write_len = strlen(http_unauthorized_response);
                    continue;
                }

                free_headers(headers);

                printf("client fd: %d is authorized\n", fd);

                // choose target to forward data to based on some load balancing logic
                int target_fd = get_optimal_target_fd(conn_idx);

                // forward data and wait for response
                // TODO: add target_fd response also to epoll for better scaling and cleaner code
                if (forward_data(conn_idx, target_fd, len) < 0) {
                    printf("forwarding data to target fd: %d failed, service down\n", target_fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, target_fd, NULL);
                    close(target_fd);
                    exit(EXIT_FAILURE);
                }

                // get response, modify client fd to epollout to write 
                /*len = read(target_fd, conn_bufs[conn_idx].write_buffer, sizeof(conn_bufs[conn_idx].write_buffer));
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
                continue;*/
                ev.events = EPOLLIN;
                ev.data.fd = target_fd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, target_fd, &ev);
            
            // target server responds
            }else if(is_target_fd(fd) && (events[i].events & EPOLLIN) ){
                int t_idx = -1;
                for(int j=0; j<TARGET_COUNT; j++){
                    if(target_fds[j]==fd){
                        t_idx = j;
                        break;
                    }
                }
                if(t_idx<0){
                    continue;
                }
                int c_idx = target_client[t_idx];
                int len = read(fd, conn_bufs[c_idx].write_buffer, BUFFER_SIZE);
                if(len<0){
                    printf("could not read from target fd: %d\n", fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                    exit(EXIT_FAILURE);
                }
                else{
                    conn_bufs[c_idx].write_len = len;
                    //schedule client_fd for write-back
                    events[i].events = EPOLLIN | EPOLLOUT;
                    events[i].data.fd = conn_bufs[c_idx].fd;
                    epoll_ctl(epfd, EPOLL_CTL_MOD, conn_bufs[c_idx].fd, &events[i]);
                }
                epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                close(fd);

            // respond to client req
            } else if (is_client_fd(fd) && (events[i].events & EPOLLOUT) ) {

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

