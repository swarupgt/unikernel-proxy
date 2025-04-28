#ifndef UNIKERNEL_PROXY_AUTH_HANDLER_H
#define UNIKERNEL_PROXY_AUTH_HANDLER_H

#define AUTH_HEADER "X-Auth-Token"

int is_authorized(char** headers);

#endif