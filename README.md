# TFTP Server Implementation

## Overview
This project implements a Trivial File Transfer Protocol (TFTP) server in C. TFTP is a simple, lightweight protocol that allows for basic file transfers over UDP, commonly used in environments where minimal setup is required, such as in firmware and boot environments.

This `README` has been enhanced to provide clearer usage instructions, setup details, and functionality explanations.

## Project Structure
- **server.c**: Contains the main TFTP server logic, including handling Read Requests (RRQ) and Write Requests (WRQ).
- **Makefile**: Automates the build process for the server.
- **README.md**: Documentation for the project, including setup and usage instructions.

## Setup and Compilation
To compile the server, use the following command:
```bash
make
