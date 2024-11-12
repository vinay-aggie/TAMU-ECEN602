# Simple HTTP Proxy Server

## Overview
This is a basic HTTP server designed to handle incoming HTTP requests and respond with a "Hello, World!" message. It demonstrates the setup of socket connections, parsing of HTTP messages, and error handling.

## Files
- **httpUtils.h**: Contains helper functions for parsing HTTP requests and extracting headers and body content.
- **server.cpp**: The main server code, which accepts incoming client connections and sends a basic HTTP response.

## Modifications
1. **Enhanced Error Handling**: Improved logging for malformed requests and connection failures.
2. **HTTP Parsing**: Updated `parseHttp` function in `httpUtils.h` for more reliable parsing of headers and body content.
3. **Resource Management**: Ensured proper closure of client sockets in error scenarios.
4. **Comments and Documentation**: Added comments to functions for clarity and maintenance.

## Usage
1. Compile the server using g++ or any compatible C++ compiler.
   ```
   g++ -o server server.cpp
   ```
2. Run the server:
   ```
   ./server
   ```
3. Connect to the server using a tool like `curl`:
   ```
   curl http://localhost:8080
   ```
   The server will respond with "Hello, World!"

## Notes
This server is a simple HTTP proxy template and can be expanded for more complex request handling.
