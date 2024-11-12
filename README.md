# Simple HTTP Proxy Server

This project is an implementation of a simple HTTP proxy server that uses caching. The server is built with HTTP/1.0 (RFC 1945) in C++. The proxy listens for incoming HTTP GET requests from clients, caches responses, and forwards requests to the intended web server when necessary.

## Files

- `main.cpp`: Starts the proxy server with specified IP and port.
- `ProxyServer.h` and `ProxyServer.cpp`: Implements the proxy server with cache and client handling.
- `Cache.h` and `Cache.cpp`: Implements an LRU cache for storing HTTP responses.

## Usage

Compile the code with the following command:
```bash
g++ main.cpp ProxyServer.cpp Cache.cpp -o proxy
