#ifndef UNIKERNEL_PROXY_CONN_HANDLER_H
#define UNIKERNEL_PROXY_CONN_HANDLER_H

void init_conn_buf();

int connect_to_targets();

int make_socket_non_blocking(int sfd);

int create_server_socket(int port);

int forward_data(int conn_idx, int target_fd, int len); 

int is_target_fd(int fd);

int is_client_fd(int fd);

int get_lowest_conn_buf();

int get_conn_buf_from_fd(int fd);

void reset_conn_buff(int client_idx);

#endif