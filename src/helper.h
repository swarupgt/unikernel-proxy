#ifndef UNIKERNEL_PROXY_HELPER_H
#define UNIKERNEL_PROXY_HELPER_H

char** get_http_headers(char* request);

int is_authorized(char** headers);

void free_headers(char** headers); 

#endif