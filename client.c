#include <stdio.h>
//#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include "socketUtils.c"

int main(int argc, char* argv[]) {
    // Verify number of arguments is correct and notify user if not correct
    if (argc > 4) {
        printf("Too many arguments. Please only provide 4 arguments.\n");
        exit(EXIT_FAILURE);
    } else if (argc < 4) {
        printf("Too few arguments. Please provide 4 arguments.\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Starting client.\n");
    }

    char* username = argv[1];
    char* ip = argv[2];
    char* port = argv[3];

    int sock;
    struct sockaddr_in serverAddr;

    // Initialize socket and throw an error if failure
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Failed to create a socket.\n");
        exit(EXIT_FAILURE);
    }

    printf("Socket created successfully\n");

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip);
    serverAddr.sin_port = htons(atoi(port));

    // Establish connection through the socket
    int connection = connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    if (connection == -1) {
        printf("Could not connect to the server.\n");
        exit(EXIT_FAILURE);
    }

    printf("Successfully connected to server.\n");

    char usernameBuff[BUFFER_SIZE];
    uint16_t userNameSize = addAttribute(usernameBuff, 0, 2, strlen(username), username);
    int sendUsername = sendMessage(sock, 3, 2, strlen(usernameBuff), usernameBuff); 
    if (sendUsername == -1) {
        printf("Failed to send username\n");
        exit(EXIT_FAILURE);
    }

    printf("Welcome, %s\n", username);

    fd_set read_fds;
    char message[BUFFER_SIZE];

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        int max_fd = sock > STDIN_FILENO ? sock : STDIN_FILENO;
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("select error");
            exit(EXIT_FAILURE);
        }

        // Check if there is data from the server
        if (FD_ISSET(sock, &read_fds)) {
            int bytes_received = receiveMessage(sock, message, sizeof(message) - 1);
            if (bytes_received == -1) {
                printf("Disconnected from server.\n");
                close(sock);
                exit(EXIT_SUCCESS);
            }
            message[bytes_received] = '\0'; // Null-terminate the string
            printf("Message from server: %s\n", message);
        }

        // Check if there is user input
        char buff[BUFFER_SIZE];
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            if (fgets(message, sizeof(message), stdin) != NULL) {
                // Remove newline character from input
                //message[strcspn(message, "\n")] = '\0';

                uint16_t userNameSize = addAttribute(buff, 0, 2, strlen(username), username);
                uint16_t messageSize = addAttribute(buff, userNameSize + HEADER_LENGTH, 4, strlen(message), message);

                int sendData = sendMessage(sock, 3, 4, strlen(buff), buff);
            }
        }
    }

    return 0;
}