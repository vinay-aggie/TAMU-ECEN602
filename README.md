# TFTP Server

### Author
This TFTP server was implemented by ChatGPT.

## Overview
This project is a simple TFTP server in C, designed for Linux. It supports **Read Request (RRQ)** functionality over UDP, allowing multiple simultaneous client connections using `fork()`. The server includes **timeouts**, **retransmissions**, and supports **netascii** and **octet** modes for file transfer.

## Features
- **UDP-based File Transfer** with 512-byte blocks
- **Multi-Client Handling** using child processes
- **Timeouts and Retransmissions** for reliability
- **Error Handling** with TFTP protocol error messages

## Compilation & Usage
1. **Compile**:
   ```bash
   make
