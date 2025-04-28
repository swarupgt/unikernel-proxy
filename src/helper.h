#ifndef UNIKERNEL_PROXY_HELPER_H
#define UNIKERNEL_PROXY_HELPER_H

char** get_http_headers(char* request);

void free_headers(char** headers); 

#endif