# unikernel-proxy
Proxy server built on Unikraft, a unikernel

Needs the `kraft` tool installed, and change the `run.sh` build command for your system.
Right now, only forwards any data incoming to a dummy service at port 9090. Expects the service to be up before it boots up, you can have a listener with `nc -l 9090`.

## TODO
- Proxy logic (HTTP)
- Add authentication
- handle multiple instances of the service for load balancing
- Makefile into Dockerfile
- Tests