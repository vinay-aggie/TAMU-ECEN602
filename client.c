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

#define MAX_USERNAME 128
#define BUFFER_SIZE 65536

// protocol version
#define VERSION 3

// header types
#define JOIN 2
#define SEND 4
#define FWD 3

// header length
#define HEADER_LENGTH 4


int sendMessage(int socket, uint16_t version, uint16_t type, uint16_t length, const char *payload) {
    uint8_t header[HEADER_LENGTH];

    /*header[0] = (version << 7) | (type & 0x7F);
    header[1] = (length >> 8) & 0xFF;
    header[2] = length & 0xFF;*/

    header[0] = version >> 1;
    header[1] = (((version & 0x01) << 7) + (type & 0x7F));
    header[2] = length >> 8;
    header[3] = length & 0xFF;

    size_t packet_size = HEADER_LENGTH + strlen(payload);

    unsigned char* message = malloc(packet_size);

    memcpy(message, header, HEADER_LENGTH);
    memcpy(message + HEADER_LENGTH, payload, strlen(payload));

    for (uint16_t i = 0; i < HEADER_LENGTH + strlen(payload); i++) {
        printf("%i, %#x\n", i, message[i]);
    }

    ssize_t bytes_sent = send(socket, message, packet_size, 0);
    
    return bytes_sent;
}

int main(int argc, char* argv[]) {
    // Verify number of arguments is correct and notify user if not correct
    if (argc > 3) {
        printf("Too many arguments. Please only provide 3 arguments.\n");
        exit(EXIT_FAILURE);
    } else if (argc < 3) {
        printf("Too few arguments. Please provide 3 arguments.\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Starting client.\n");
    }

    char* ip = argv[1];
    char* port = argv[2];

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

    // Define the buffers used to store data for sending and receiving respectively
    //char sendBuff[BUFFER_SIZE];
    //char readBuff[BUFFER_SIZE];

    char username[MAX_USERNAME];
    printf("Enter a username\n");
    fgets(username, MAX_USERNAME, stdin);

    int sendUsername = sendMessage(sock, 3, 2, strlen(username), username); //send(sock, username, strlen(username), 0);
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
            int bytes_received = recv(sock, message, sizeof(message) - 1, 0);
            if (bytes_received <= 0) {
                printf("Disconnected from server.\n");
                close(sock);
                exit(EXIT_SUCCESS);
            }
            message[bytes_received] = '\0'; // Null-terminate the string
            printf("Message from server: %s\n", message);
        }

        // Check if there is user input
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            if (fgets(message, sizeof(message), stdin) != NULL) {
                // Remove newline character from input
                message[strcspn(message, "\n")] = '\0';

                /*uint8_t header[HEADER_LENGTH];

                int vrsn = 3;
                int type = SEND;
                uint8_t length = htonl(strlen(message));

                size_t packet_size = htonl(strlen(message)) + HEADER_LENGTH;
                char* buffer = malloc(packet_size);

                memcpy(buffer, &header, HEADER_LENGTH);*/

                int sendData = sendMessage(sock, 3, 4, strlen(message), message);

                // Send the message to the server
                if (sendData) {
                    perror("send error");
                }
            }
        }
    }


    //int sel = select(maxfdp1, readset, writeset, exceptset, NULL);

    /*while (1) {
        memset(sendBuff, 0, sizeof(sendBuff));
        memset(readBuff, 0, sizeof(readBuff));

        printf("Enter text to send to server: (Press Control-D to stop)\n");

        // Get user input and store in buffer before sending through socket
        fgets(sendBuff, BUFFER_SIZE, stdin);

        // Disconnect from server if user inputs control-D
        if (feof(stdin)) {
            printf("\nDisconnecting from server\n");

            close(sock);
            exit(0);
        }

        // Write bytes to the socket
        ssize_t writeBytes = writen(sock, sendBuff, strlen(sendBuff));
        
        // Receive message back through socket, which should be the same length as the original message
        ssize_t recBytes = readline(sock, readBuff, strlen(sendBuff));
        if (recBytes == -1) {
            printf("Failed to read data from server. Error = {%s}\n", strerror(errno));
            close(sock);
            exit(EXIT_FAILURE);
        } else if (recBytes != writeBytes) {
            printf("Message from server does not match message sent.\n");
            close(sock);
            exit(EXIT_FAILURE);
        }

        printf("Message from server: %s\n", readBuff);
    }*/

    return 0;
}