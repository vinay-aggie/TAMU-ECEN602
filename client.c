#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 512

/*******************************************************
 * writen function:
 *      Writes data of fixed size to the socket establishing communication between client and server
 * 
 * Inputs:
 *      int clientFd: socket connection between server and client
 *      const char *ptrBuf: pointer to the buffer containing text to be sent through the socket
 *      size_t bytesToWrite: size of the packet to send
 * 
 * Outputs:
 *      int: Total number of bytes written to socket
 ******************************************************/
int writen(int clientFd, const char *ptrBuf, size_t bytesToWrite)
{
    size_t bytesLeft;
    ssize_t bytesWritten;

    bytesLeft = bytesToWrite;
    while (bytesLeft > 0)
    {
        bytesWritten = send(clientFd, ptrBuf, bytesLeft, 0);
        if (bytesWritten < 0)
        {
            if (errno == EINTR)
            {
                bytesWritten = 0;
                continue;
            }
            else
            {
                return -1;
            }
        }

        bytesLeft = bytesLeft - bytesWritten;
        ptrBuf = ptrBuf + bytesWritten;
    }

    // Ideally this should be 0;
    return (int)bytesWritten;
}

/* ******************************************************
 * readline function:
 *      Reads data of fixed size coming through socket
 * 
 * Inputs:
 *      int fd: socket connection between server and client
 *      const char *ptrBuf: pointer to the buffer to be filled with text sent through the socket
 *      size_t bytesToRead: size of the packet that was sent
 * 
 * Outputs:
 *      int: Total number of bytes read from the socket
 ****************************************************** */
int readline(int fd, char *ptrBuff, size_t bytesToRead) {
    int bytesLeft;
    int bytesRead;

    bytesLeft = bytesToRead;
    while (bytesLeft > 0) {
        if ((bytesRead = read(fd, ptrBuff, bytesLeft)) < 0) {
            if (errno == EINTR) {
                bytesRead = 0;
            } else {
                return -1;
            }
        } else if (bytesRead == 0) {
            break;
        }

        bytesLeft -= bytesRead;
        ptrBuff += bytesRead;
    }

    return bytesToRead - bytesLeft;
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
        printf("Failed to create a socket.\n{%s}\n", strerror(errno));
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
    char sendBuff[BUFFER_SIZE];
    char readBuff[BUFFER_SIZE];

    while (1) {
        memset(sendBuff, 0, sizeof(sendBuff));
        memset(readBuff, 0, sizeof(readBuff));

        printf("Enter text to send to server: (Press Control-D to stop)\n");

        // Get user input and store in buffer before sending through socket
        char* input = fgets(sendBuff, BUFFER_SIZE, stdin);

        // Disconnect from server if user inputs control-D
        if (input == NULL) {
            printf("Disconnecting from server\n");

            close(sock);
            exit(0);
        }

        // Write bytes to the socket
        int writeBytes = writen(sock, input, strlen(input));
        if (writeBytes != strlen(input)) {
            printf("Failed to send data to server. Error = {%s}\n", strerror(errno));
            close(sock);
            exit(EXIT_FAILURE);
        }
        
        // Receive message back through socket, which should be the same length as the original message
        int recBytes = readline(sock, readBuff, strlen(input));
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
    }

    return 0;
}
