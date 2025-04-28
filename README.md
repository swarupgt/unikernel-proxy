# Unikernel Proxy

In this project, we present an reverse proxy written and built for [unikraft](https://unikraft.org/). A reverse proxy is a service used to protect the identity of the servers from externel clients. We aim to provide authentication and load-balancing capabilities in this reverse proxy. The flow of this system will look like this: 

![rev-proxy](./docs/rev_proxy.png)

The clients on the left will only be able to communicate to the subsystem on the right via the reverse proxy (as only that port and host is exposed to the clients). 

Any request from the client must contain the authentication header 'X-Auth-Token'. 
Once the request is authenticated, the load balancing logic selects one of the servers available to the proxy and creates a connection with them. It then forwards the request from client to the server. After the server responds, the response is then read and sent to the client via client's fd. This way, the client never maintains a direct connection with the server. 


## Group Members
- Ruturaj Shitole
- Boxin Yang
- Swarup Totloor
- Prince Noah Johnson

## Description
A simple reverse proxy server written in C built on Unikraft, a unikernel. A client can fire a GET request to the proxy server IP. The server authenticates the incoming request from its headers, and uses a load balancing algorithm (round robin in this case) to route this request to the appropriate target server node. The target server responds, and the the proxy forwards the response back to the client, all without the client knowing anything about the internal server nodes. 

The target servers in our case are just simple Python HTTP servers. These are separate from our actual project, and run externally.

For more detailed information on the implementation, please refer to the following files:
- [Core server loop](./src/main.c)
- [Load balancing](./src/load_balancer.c)
- [Auth implementation](./src/auth_handler.c)
- [Tests](./tests/)

## Exploring Go for our Unikraft project
We initially planned to write our proxy in Go, since apparently unikraft supports Go (both from kraftkit and building from source). Our (unsuccessful) attempts working with both methods are documented more in detail [here](./REPORT.md). Due to timing constraints, we decided to implement our project in C instead. 

## Running the project

This project needs the `kraft` tool installed beforehand. You can install it from [here](https://github.com/unikraft/kraftkit?tab=readme-ov-file#installation)

We also need Docker installed.

To run our project, 
1. Run the `run_services.sh` script to start simple `Hello World` servers on ports `9090` through `9099`.
2. Run the `run.sh` script to start the proxy server on port `8080`.
3. Send a GET request to the proxy server with the `X-Auth-Token` header. For example - 
    ```bash
        curl -H "X-Auth-Token: token" localhost:8080
    ```

## Tests

To run the unit tests for this project run the following command: 

```shell
make test
```

The expected output should be:

```shell
---------RUNNING TESTS---------
test:03_test_helper:test_get_http_headers:PASSED
test:03_test_helper:test_free_headers:PASSED
test:01_test_auth_handler:test_unauthorized:PASSED
test:01_test_auth_handler:test_authorized:PASSED
test:04_test_load_balancer:test_round_robin_allocation:PASSED
test:02_test_conn_handler:test_init_conn_buf:PASSED
test:02_test_conn_handler:test_is_client_fd:PASSED
test:02_test_conn_handler:test_is_target_fd:PASSED
---------FINISHED TESTS---------
```

