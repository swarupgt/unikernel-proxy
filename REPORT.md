# Unikernel Proxy

In this project, we present an reverse proxy written and built for [unikraft](https://unikraft.org/). A reverse proxy is a service used to protect the identity of the servers from externel clients. We aim to provide authentication and load-balancing capabilities in this reverse proxy. The flow of this system will look like this: 

![rev-proxy](./docs/rev_proxy.png)

The clients on the left will only be able to communicate to the subsystem on the right via the reverse proxy (as only that port and host is exposed to the clients). 

Any request from the client must contain the authentication header 'X-Auth-Token'. 
Once the request is authenticated, the load balancing logic selects one of the servers available to the proxy and creates a connection with them. It then forwards the request from client to the server. After the server responds, the response is then read and sent to the client via client's fd. This way, the client never maintains a direct connection with the server. 


## How does the project build work?

Unikraft provides a tool called [Kraftkit](https://github.com/unikraft/kraftkit) for building the unikernel with a minimal filesystem using Docker. It uses Dockerfile with multi-stage builds to build the source application in the earlier stages and copies the build files from them into the last stage where the base image is `scratch`, a minimal image to run things. Once the build is done, the kraft tool copies the filesystem from the docker build into the .unikernel directory. These details can be found in the Dockerfile and the Kraftfile. 


## Our explorations for Go support in Unikraft 

The initial idea was to develop a build of unikraft that contains support for running Go programs and then use that to build the reverse proxy. The benefit of using this approach was to use the capabilities of Goroutines and fastpaths to better manage the working of the entire proxy. 

After a series of discussions with Jacob and others in the Discord channel, we concluded that using Go might be not fully supported for general use in Unikraft. 

### **The provided examples are outdated**
We started with the [http-go](https://github.com/unikraft/catalog/tree/main/examples/http-go1.21) example from the unikraft examples catalog. This example uses Kraftkit for building a minimal http server in Go using native library support. Though this seemed simple to work with on the first look, once we tried it out, it failed with an error saying 'pthread' is not supported. This was a major bummer for us as Goroutines use pthreads. 

### **Compilation fails when we try to build**


### **Without Kraftkit: app-helloworld-go, does not work**
https://github.com/unikraft/app-helloworld-go 
'0.6'?
libs in wrong order in kraft.yaml
compilation fails


### **Some examples do work, but they do not have much documentation on how to repurpose them**
