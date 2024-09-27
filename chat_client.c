#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int sock = 0, valread;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE] = {0};
    fd_set readfds;

    if (argc != 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return -1;
    }

    // Create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));

    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        printf("Invalid address\n");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    printf("Connected to server. Type your message:\n");

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        // Wait for activity on stdin or socket
        if (select(sock + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("Select error");
            break;
        }

        // Check for input from the user
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            fgets(buffer, BUFFER_SIZE, stdin);
            send(sock, buffer, strlen(buffer), 0);
        }

        // Check for message from the server
        if (FD_ISSET(sock, &readfds)) {
            if ((valread = read(sock, buffer, BUFFER_SIZE)) == 0) {
                printf("Server disconnected\n");
                close(sock);
                exit(EXIT_SUCCESS);
            }
            buffer[valread] = '\0';
            printf("Server: %s", buffer);
        }
    }

    return 0;
}
