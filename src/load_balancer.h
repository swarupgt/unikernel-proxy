#ifndef UNIKERNEL_PROXY_LOAD_BALANCER_H
#define UNIKERNEL_PROXY_LOAD_BALANCER_H


/*

    The load balancing logic. 
    This code does a Round Robin scheduling for target FDs.
    It adds the connection index to the target's client.

    params:
        conn_idx:
            the connection index for the requesting client

    return:
        the file descriptor the target server which should 
        serve the request 

*/
int get_optimal_target_fd(int conn_idx); 

#endif