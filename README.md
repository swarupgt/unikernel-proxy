# unikernel-proxy

## Group Members
- Ruturaj Shitole
- Boxin Yang
- Swarup Totloor
- Prince Noah Johnson

## Description
A simple reverse proxy server written in C built on Unikraft, a unikernel. A client can fire a GET request to the proxy server IP. The server authenticates the incoming request from its headers, and uses a load balancing algorithm (round robin in this case) to route this request to the appropriate target server node. The target server responds, and the the proxy forwards the response back to the client, all without the client knowing anything about the internal server nodes. 

For more detailed information on the implementation, please refer to the following files:
- [Core server loop](./src/main.c)
- [Load balancing](./src/load_balancer.c)
- [Auth implementation](./src/auth_handler.c)
- [Tests](./tests/)

## Exploring Go for our Unikraft project
We initially planned to write our proxy in Go, since apparently unikraft supports Go (both from kraftkit and building from source). Our (unsuccessful) attempts working with both methods are documented more in detail [here](./REPORT.md). Due to timing constraints, we decided to implement our project in C instead. 

## Running the project

This project needs the `kraft` tool installed beforehand. You can install it from [here](https://github.com/unikraft/kraftkit?tab=readme-ov-file#installation)

To run our project, 
1. Run the `run_services.sh` script to start simple `Hello World` servers on ports `9090` through `9099`.
2. Run the `run.sh` script to start the proxy server on port `8080`.
3. Send a GET request to the proxy server with the `X-Auth-Token` header. For example - 
    ```bash
        curl -H "X-Auth-Token: token" localhost:8080
    ```

## Tests