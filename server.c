// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

#define SERVER_PORT 12345  // Default port for TFTP server
#define BUFFER_SIZE 516    // 512 bytes for data, 4 bytes for opcode and block number
#define DATA_SIZE 512
#define TIMEOUT_INTERVAL 1 // Timeout in seconds
#define MAX_RETRIES 10     // Max retries before terminating transfer

#define OPCODE_RRQ 1
#define OPCODE_DATA 3
#define OPCODE_ACK 4
#define OPCODE_ERROR 5

void handle_client(int sock, struct sockaddr_in client_addr, socklen_t client_len, char *filename, char *mode);
void send_error(int sock, struct sockaddr_in *client_addr, socklen_t client_len, int error_code, const char *error_msg);

int main() {
    int sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    ssize_t recv_len;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("TFTP Server is listening on port %d\n", SERVER_PORT);

    while (1) {
        client_len = sizeof(client_addr);
        recv_len = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);

        if (recv_len < 0) {
            perror("Error receiving data");
            continue;
        }

        if (fork() == 0) { // Child process handles the client
            char filename[256], mode[12];
            int filename_len = strlen(buffer + 2);
            int mode_len = strlen(buffer + 3 + filename_len);

            strcpy(filename, buffer + 2);
            strcpy(mode, buffer + 3 + filename_len);

            close(sock); // Child doesn't need the listening socket
            int child_sock = socket(AF_INET, SOCK_DGRAM, 0);
            handle_client(child_sock, client_addr, client_len, filename, mode);
            close(child_sock);
            exit(0);
        }
    }
    close(sock);
    return 0;
}

void handle_client(int sock, struct sockaddr_in client_addr, socklen_t client_len, char *filename, char *mode) {
    FILE *file;
    char buffer[BUFFER_SIZE];
    char data_buffer[DATA_SIZE];
    int block_number = 1, bytes_read, retries;
    struct timeval timeout = {TIMEOUT_INTERVAL, 0};
    fd_set fds;
    ssize_t recv_len;

    file = fopen(filename, (strcasecmp(mode, "netascii") == 0) ? "r" : "rb");
    if (!file) {
        send_error(sock, &client_addr, client_len, 1, "File not found");
        return;
    }

    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);

    while (1) {
        // Prepare the DATA packet
        buffer[0] = 0;
        buffer[1] = OPCODE_DATA;
        buffer[2] = (block_number >> 8) & 0xFF;
        buffer[3] = block_number & 0xFF;
        bytes_read = fread(data_buffer, 1, DATA_SIZE, file);
        memcpy(buffer + 4, data_buffer, bytes_read);

        // Send DATA packet
        if (sendto(sock, buffer, bytes_read + 4, 0, (struct sockaddr *)&client_addr, client_len) < 0) {
            perror("Error sending data packet");
            break;
        }

        // Wait for ACK
        retries = 0;
        while (retries < MAX_RETRIES) {
            FD_ZERO(&fds);
            FD_SET(sock, &fds);

            int select_result = select(sock + 1, &fds, NULL, NULL, &timeout);
            if (select_result < 0) {
                perror("Error on select()");
                break;
            } else if (select_result == 0) {
                // Timeout
                if (sendto(sock, buffer, bytes_read + 4, 0, (struct sockaddr *)&client_addr, client_len) < 0) {
                    perror("Error resending data packet");
                    break;
                }
                retries++;
            } else {
                recv_len = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
                if (recv_len >= 4 && buffer[0] == 0 && buffer[1] == OPCODE_ACK) {
                    int ack_block_number = (buffer[2] << 8) | buffer[3];
                    if (ack_block_number == block_number) break;
                }
            }
        }

        if (retries == MAX_RETRIES) {
            printf("Transfer timed out.\n");
            break;
        }

        if (bytes_read < DATA_SIZE) {
            printf("File transfer completed.\n");
            break; // Last packet
        }
        block_number++;
    }
    fclose(file);
}

void send_error(int sock, struct sockaddr_in *client_addr, socklen_t client_len, int error_code, const char *error_msg) {
    char buffer[BUFFER_SIZE];
    buffer[0] = 0;
    buffer[1] = OPCODE_ERROR;
    buffer[2] = (error_code >> 8) & 0xFF;
    buffer[3] = error_code & 0xFF;
    strcpy(buffer + 4, error_msg);
    sendto(sock, buffer, strlen(error_msg) + 5, 0, (struct sockaddr *)client_addr, client_len);
}
