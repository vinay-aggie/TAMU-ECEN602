
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include "socketUtils.h"
#include "sbcpProtocol.h"

void handle_error(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <username> <server IP> <server port>\\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* username = argv[1];
    char* ip = argv[2];
    char* port = argv[3];

    // Validate port number
    int port_num = atoi(port);
    if (port_num <= 0 || port_num > 65535) {
        fprintf(stderr, "Invalid port number.\\n");
        exit(EXIT_FAILURE);
    }

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        handle_error("Failed to create socket");
    }
    printf("Socket created successfully.\\n");

    // Configure server address
    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip);
    serverAddr.sin_port = htons(port_num);

    // Connect to server
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        handle_error("Failed to connect to server");
    }
    printf("Connected to server successfully.\\n");

    // Prepare username message
    char usernameBuff[BUFFER_SIZE];
    addAttribute(usernameBuff, 0, USERNAME, strlen(username), username);

    // Send username to server
    if (sendMessage(sock, VERSION, JOIN, HEADER_LENGTH + strlen(username), usernameBuff) == -1) {
        handle_error("Failed to send username");
    }
    printf("Welcome, %s\\n", username);

    fd_set read_fds;
    char message[BUFFER_SIZE];

    // Main loop to handle server communication
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        if (select(sock + 1, &read_fds, NULL, NULL, NULL) < 0) {
            handle_error("select() failed");
        }

        // Handle input from user
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            if (fgets(message, BUFFER_SIZE, stdin) != NULL) {
                if (sendMessage(sock, VERSION, SEND, strlen(message), message) == -1) {
                    handle_error("Failed to send message");
                }
            }
        }

        // Handle message from server
        if (FD_ISSET(sock, &read_fds)) {
            uint16_t version, type, length;
            if (receiveMessage(sock, message, &version, &type, &length) == -1) {
                handle_error("Failed to receive message");
            }
            message[length] = '\\0';  // Ensure null-terminated string
            printf("Received message: %s\\n", message);
        }
    }

    close(sock);
    return 0;
}
'''

# Enhanced socketUtils.h code
enhanced_socket_utils_code = '''\
// Header file.

#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <stdio.h>
#include <stdint.h>

#define MAX_USERNAME 128
#define BUFFER_SIZE 1024

// protocol version
#define VERSION 3

// header types
#define JOIN 2
#define SEND 4
#define FWD 3

// header length
#define HEADER_LENGTH 4

#define MAX_CONNECTIONS 100

#define USERNAME 2
#define MESSAGE 4
#define REASON 1
#define CLIENT_COUNT 3

// Utility functions for socket handling
void addAttribute(char* buff, uint16_t offset, uint16_t type, uint16_t length, const char* payload);
int sendMessage(int socket, uint16_t version, uint16_t type, uint16_t length, const char *payload);
int receiveMessage(int fd, char *buf, uint16_t* _version, uint16_t* _type, uint16_t* _length);
void readAttribute(char *readBuff, char* writeBuff, uint16_t offset, uint16_t* _type, uint16_t* _length);

#endif /* SOCKET_UTILS_H */
'''

# Enhanced server.c code with better readability and structure
enhanced_server_code = '''\
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "socketUtils.h"
#include "sbcpProtocol.h"

void handle_error(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port_num = atoi(argv[1]);
    if (port_num <= 0 || port_num > 65535) {
        fprintf(stderr, "Invalid port number.\\n");
        exit(EXIT_FAILURE);
    }

    int serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketFd == -1) {
        handle_error("Failed to create socket");
    }

    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port_num);

    if (bind(serverSocketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        handle_error("Failed to bind socket");
    }

    if (listen(serverSocketFd, MAX_CONNECTIONS) == -1) {
        handle_error("Failed to listen on socket");
    }

    printf("Server listening on port %d\\n", port_num);

    fd_set active_fds, read_fds;
    FD_ZERO(&active_fds);
    FD_SET(serverSocketFd, &active_fds);

    while (1) {
        read_fds = active_fds;
        if (select(FD_SETSIZE, &read_fds, NULL, NULL, NULL) < 0) {
            handle_error("select() failed");
        }

        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == serverSocketFd) {
                    // Accept new connection
                    struct sockaddr_in clientAddr = {0};
                    socklen_t clientLen = sizeof(clientAddr);
                    int clientSocketFd = accept(serverSocketFd, (struct sockaddr*)&clientAddr, &clientLen);
                    if (clientSocketFd == -1) {
                        handle_error("Failed to accept client");
                    }
                    FD_SET(clientSocketFd, &active_fds);
                    printf("New client connected: %d\\n", clientSocketFd);
                } else {
                    // Handle client message
                    char buffer[BUFFER_SIZE];
                    uint16_t version, type, length;
                    if (receiveMessage(i, buffer, &version, &type, &length) == -1) {
                        close(i);
                        FD_CLR(i, &active_fds);
                        printf("Client disconnected: %d\\n", i);
                    } else {
                        printf("Message from client %d: %s\\n", i, buffer);
                        // Forward message to other clients
                        for (int j = 0; j < FD_SETSIZE; j++) {
                            if (FD_ISSET(j, &active_fds) && j != serverSocketFd && j != i) {
                                if (sendMessage(j, VERSION, FWD, length, buffer) == -1) {
                                    handle_error("Failed to forward message");
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    close(serverSocketFd);
    return 0;
}
