# ECEN-602-Machine-Problems

# SBCP Protocol Client-Server Application

## Overview

This project is a client-server application that communicates using the **SBCP (Simple Broadcast Communication Protocol)** over TCP. The server accepts multiple client connections, and clients can send messages to the server, which then broadcasts those messages to all other connected clients.

## File Structure

- `client.c` – The client-side program. It connects to the server and allows users to send and receive messages.
- `server.c` – The server-side program. It handles multiple client connections and forwards messages between clients.
- `socketUtils.h` – Contains utility functions for managing socket communication.
- `sbcpProtocol.h` – Defines constants and structures related to the SBCP protocol.
- `Makefile` – A build script for compiling the client and server programs.

## Dependencies

- A C compiler like `gcc` is required to build the programs.
- Standard libraries for sockets (`arpa/inet.h`, `sys/socket.h`) are used.
  
## How to Build

You can compile both the client and server using the provided `Makefile`.

1. **To build both client and server:**
   ```bash
   make
