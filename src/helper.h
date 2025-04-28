#ifndef UNIKERNEL_PROXY_HELPER_H
#define UNIKERNEL_PROXY_HELPER_H

/*

    Get the HTTP headers from the request object.

    params: 
        request:
            the entire request object
    
    return: 
        a list of strings containing the headers 

*/
char** get_http_headers(char* request);


/*

    Free the headers array. 

    params: 
        headers: 
            an array of strings containing the headers

    return: None

*/
void free_headers(char** headers); 

#endif