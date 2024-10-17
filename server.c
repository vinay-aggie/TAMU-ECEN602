#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <time.h>

// Constants
#define BUFFER_SIZE 516
#define DATA_SIZE 512
#define ACK_SIZE 4

// TFTP Protocol Opcodes
#define TFTP_RRQ    1
#define TFTP_WRQ    2
#define TFTP_DATA   3
#define TFTP_ACK    4
#define TFTP_ERROR  5

// Error Codes
#define FILE_NOT_FOUND 1
#define ACCESS_VIOLATION 2
#define DISK_FULL 3
#define ILLEGAL_OPERATION 4
#define UNKNOWN_TID 5
#define FILE_EXISTS 6
#define NO_SUCH_USER 7

// Function Prototypes
void handleRRQ(int sockfd, struct sockaddr_in *client, socklen_t clientlen, char *filename);
void handleWRQ(int sockfd, struct sockaddr_in *client, socklen_t clientlen, char *filename);
void sendError(int sockfd, struct sockaddr_in *client, socklen_t clientlen, int errorCode, const char *errorMessage);
void logActivity(const char *message);

// Main function for server operation
int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientlen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];

    // Signal handler to catch Ctrl+C
    signal(SIGINT, [](int signo) {
        printf("\nShutting down server...\n");
        exit(0);
    });

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Server address setup
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(69);

    // Bind socket
    if (bind(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    logActivity("TFTP server started...");

    // Main loop to handle incoming requests
    while (1) {
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &clientlen);
        if (n < 0) {
            perror("Receive failed");
            continue;
        }
        logActivity("Received request");

        // Check operation type (RRQ or WRQ)
        int opcode = (buffer[0] << 8) | buffer[1];
        char *filename = buffer + 2;

        switch (opcode) {
            case TFTP_RRQ:
                handleRRQ(sockfd, &clientAddr, clientlen, filename);
                break;
            case TFTP_WRQ:
                handleWRQ(sockfd, &clientAddr, clientlen, filename);
                break;
            default:
                sendError(sockfd, &clientAddr, clientlen, ILLEGAL_OPERATION, "Unsupported operation");
                break;
        }
    }

    close(sockfd);
    return 0;
}

// Function to handle Read Request (RRQ)
void handleRRQ(int sockfd, struct sockaddr_in *client, socklen_t clientlen, char *filename) {
    char data[DATA_SIZE + 4];
    int file = open(filename, O_RDONLY);
    if (file < 0) {
        logActivity("File not found for RRQ");
        sendError(sockfd, client, clientlen, FILE_NOT_FOUND, "File not found");
        return;
    }

    int block = 1, n;
    do {
        n = read(file, data + 4, DATA_SIZE);
        if (n < 0) {
            logActivity("Error reading file");
            close(file);
            return;
        }
        data[0] = 0; data[1] = TFTP_DATA;
        data[2] = (block >> 8) & 0xFF;
        data[3] = block & 0xFF;
        sendto(sockfd, data, n + 4, 0, (struct sockaddr *)client, clientlen);
        block++;
    } while (n == DATA_SIZE);

    logActivity("File transfer complete");
    close(file);
}

// Function to handle Write Request (WRQ)
void handleWRQ(int sockfd, struct sockaddr_in *client, socklen_t clientlen, char *filename) {
    char ack[ACK_SIZE] = {0, TFTP_ACK, 0, 0};
    int file = open(filename, O_WRONLY | O_CREAT, 0666);
    if (file < 0) {
        logActivity("File access error for WRQ");
        sendError(sockfd, client, clientlen, ACCESS_VIOLATION, "Cannot write to file");
        return;
    }

    sendto(sockfd, ack, ACK_SIZE, 0, (struct sockaddr *)client, clientlen);
    logActivity("WRQ accepted, writing file");

    char buffer[BUFFER_SIZE];
    int n, block = 0;
    do {
        n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)client, &clientlen);
        if (n < 0) break;
        write(file, buffer + 4, n - 4);
        ack[2] = (block >> 8) & 0xFF;
        ack[3] = block & 0xFF;
        sendto(sockfd, ack, ACK_SIZE, 0, (struct sockaddr *)client, clientlen);
        block++;
    } while (n == DATA_SIZE + 4);

    logActivity("File write complete");
    close(file);
}

// Function to send an error packet
void sendError(int sockfd, struct sockaddr_in *client, socklen_t clientlen, int errorCode, const char *errorMessage) {
    char errorPacket[BUFFER_SIZE] = {0, TFTP_ERROR, (errorCode >> 8) & 0xFF, errorCode & 0xFF};
    strcpy(errorPacket + 4, errorMessage);
    sendto(sockfd, errorPacket, 4 + strlen(errorMessage) + 1, 0, (struct sockaddr *)client, clientlen);
}

// Function to log server activity
void logActivity(const char *message) {
    time_t now = time(NULL);
    char *timeStr = ctime(&now);
    timeStr[strlen(timeStr) - 1] = 0;  // Remove newline
    printf("[%s] %s\n", timeStr, message);
}
