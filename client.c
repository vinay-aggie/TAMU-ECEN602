#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "socketUtils.h"

#define BUFFER_SIZE 512

void handle_error(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
    // Check the number of arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP_ADDRESS> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* ip = argv[1];
    char* port = argv[2];

    int sock;
    struct sockaddr_in serverAddr;

    // Initialize socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        handle_error("Socket creation failed");
    }
    printf("Socket created successfully\n");

    // Configure server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip);
    serverAddr.sin_port = htons(atoi(port));

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        handle_error("Connection failed");
    }
    printf("Successfully connected to server.\n");

    char sendBuff[BUFFER_SIZE];
    char readBuff[BUFFER_SIZE];

    while (1) {
        memset(sendBuff, 0, sizeof(sendBuff));
        memset(readBuff, 0, sizeof(readBuff));

        printf("Enter text to send to server: (Press Control-D to stop)\n");

        // Get user input
        if (fgets(sendBuff, BUFFER_SIZE, stdin) == NULL) {
            if (feof(stdin)) {
                printf("End of input. Disconnecting from server.\n");
            } else {
                handle_error("Error reading from stdin");
            }
            close(sock);
            exit(EXIT_SUCCESS);
        }

        // Write data to the socket
        ssize_t writeBytes = writen(sock, sendBuff, strlen(sendBuff));
        if (writeBytes != strlen(sendBuff)) {
            handle_error("Failed to send data to server");
        }

        // Read response from the server
        ssize_t recBytes = readline(sock, readBuff, sizeof(readBuff) - 1);
        if (recBytes == -1) {
            handle_error("Failed to read data from server");
        }

        // Output the server's response
        printf("Message from server: %s\n", readBuff);
    }

    // Clean up
    close(sock);
    return 0;
}
