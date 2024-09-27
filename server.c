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
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port_num = atoi(argv[1]);
    if (port_num <= 0 || port_num > 65535) {
        fprintf(stderr, "Invalid port number.\n");
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

    printf("Server listening on port %d\n", port_num);

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
                    printf("New client connected: %d\n", clientSocketFd);
                } else {
                    // Handle client message
                    char buffer[BUFFER_SIZE];
                    uint16_t version, type, length;
                    if (receiveMessage(i, buffer, &version, &type, &length) == -1) {
                        close(i);
                        FD_CLR(i, &active_fds);
                        printf("Client disconnected: %d\n", i);
                    } else {
                        printf("Message from client %d: %s\n", i, buffer);
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
