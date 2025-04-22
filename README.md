# unikernel-proxy
Simple reverse proxy server built on Unikraft, a unikernel.

Needs the `kraft` tool installed, and change the `run.sh` build command for your system.

- Run the `service.py` app, this is a dummy service that runs on `:9090`. 
- Proxy connects to target for every connection, so it's stateless. 
- Target fd is blocking for now.
- Add `X-Auth-Token: token` as a header to check for auth.

## TODO
- handle multiple instances of the service for load balancing
- add non blocking target fd into epoll for scaling
- current code is a mess, restructure and add makefile into dockerfile
- tests
- logs