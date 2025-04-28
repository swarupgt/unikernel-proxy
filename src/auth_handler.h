#ifndef UNIKERNEL_PROXY_AUTH_HANDLER_H
#define UNIKERNEL_PROXY_AUTH_HANDLER_H

#define AUTH_HEADER "X-Auth-Token"


/*
    The authorization logic. We provide the parsed headers from the 
    request and the the function is responsible for handling the 
    authentication logic. 

    params:
        headers
            a list of strings containing the headers
    
    return:
        1 -> client is authorized
        0 -> client is not authorized
*/
int is_authorized(char** headers);

#endif