#ifndef UNIKERNEL_PROXY_CONN_HANDLER_H
#define UNIKERNEL_PROXY_CONN_HANDLER_H


/*

    Set the default values of the data structures used for 
    tracking the connections and targets. 

    params: None

    return: None

*/
void init_conn_buf();


/*

    Creates a connection to a target server on the given port. 
    Sets the target's socket to non-blocking.

    params:
        port:
            the port where the target is listening
    
    return:
        the file descriptor of the target connection

*/
int connect_to_targets(int port);


/*

    Makes the given socket non-blocking by adding the O_NONBLOCK flag.

    params:
        sfd: 
            the socket file descriprtor to set non-blocking
    
    return:
        the updated file descriptor

*/
int make_socket_non_blocking(int sfd);


/*
    Creates the server socket on the provided port and starts listening.

    params:
        port:
            the port the server should listen on
    
    return:
        the file descriptor of the server's socket 

*/
int create_server_socket(int port);


/*

    Write the data from a connection's read_buffer to target.

    params:
        conn_idx:
            the index of the connection object in the conn_bufs 

        target_fd:
            the file descriptor of the target server

        len:
            the length of the data 
        
    return: 
        0  -> success
        -1 -> error 

*/
int forward_data(int conn_idx, int target_fd, int len); 


/*

    Check if the provided file descriptor is a target fd. 

    params:
        fd: 
            the file descriptor to check for
    
    return:
        0 -> not a target fd
        1 -> is a target fd

*/
int is_target_fd(int fd);


/*

    Check if the provided file descriptor is a client fd. 

    params:
        fd: 
            the file descriptor to check for
    
    return:
        0 -> not a client fd
        1 -> is a client fd

*/
int is_client_fd(int fd);


/*

    Return the lowest available connection buffer. 

    params: None

    return: 
        the index of the lowest available connection buffer 

*/
int get_lowest_conn_buf();


/*

    Returns the connection buffer corresponding to a file descripror. 

    params:
        fd: 
            the file descriptor to check for

    return: 
        the index of the connection object in the conn_buf array
        -1 otherwise

*/
int get_conn_buf_from_fd(int fd);


/*

    Resets the connection buff to default values. 

    params:
        client_idx:
            the client index containing the conn_buffer 
    
    return: None

*/
void reset_conn_buff(int client_idx);

#endif