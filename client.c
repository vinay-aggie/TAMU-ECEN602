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
        fprintf(stderr, "Usage: %s <username> <server IP> <server port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* username = argv[1];
    char* ip = argv[2];
    char* port = argv[3];

    // Validate port number
    int port_num = atoi(port);
    if (port_num <= 0 || port_num > 65535) {
        fprintf(stderr, "Invalid port number.\n");
        exit(EXIT_FAILURE);
    }

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        handle_error("Failed to create socket");
    }
    printf("Socket created successfully.\n");

    // Configure server address
    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip);
    serverAddr.sin_port = htons(port_num);

    // Connect to server
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        handle_error("Failed to connect to server");
    }
    printf("Connected to server successfully.\n");

    // Prepare username message
    char usernameBuff[BUFFER_SIZE];
    addAttribute(usernameBuff, 0, USERNAME, strlen(username), username);

    // Send username to server
    if (sendMessage(sock, VERSION, JOIN, HEADER_LENGTH + strlen(username), usernameBuff) == -1) {
        handle_error("Failed to send username");
    }
    printf("Welcome, %s\n", username);

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
            message[length] = '\0';  // Ensure null-terminated string
            printf("Received message: %s\n", message);
        }
    }

    close(sock);
    return 0;
}
