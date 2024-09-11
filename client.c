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

ssize_t readn(int fd, char *ptrBuff, size_t bytesToRead) {
    size_t bytesLeft;
    ssize_t bytesRead;

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

ssize_t readline(int fd, char *ptr, size_t maxlen) {
    ssize_t n, rc;
    char c;

    for (n = 1; n < maxlen; n++) {
        again:
            if ((rc = read(fd, &c, 1)) == 1) {
                *ptr++ = c;
                if (c == '\n')
                    break;
            } else if (rc == 0) {
                *ptr = 0;
                return (n - 1);
            } else {
                if (errno == EINTR) {
                    goto again;
                }
                return -1;
            }
    }

    *ptr = 0;
    return n;
}

char* convertToString(char* a, int size) {
    int i;
    char* s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}

int main(int argc, char* argv[]) {
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

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Failed to create a socket.\n{%s}\n", strerror(errno));
        exit(EXIT_FAILURE);        
    }

    printf("Socket created successfully\n");

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip);
    serverAddr.sin_port = htons(atoi(port));

    int connection = connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    if (connection == -1) {
        printf("Could not connect to the server.\n");
        exit(EXIT_FAILURE);
    }

    printf("Successfully connected to server.\n");

    char sendBuff[BUFFER_SIZE];
    char readBuff[BUFFER_SIZE];

    while (1) {
        printf("Enter text to send to server: (Press Control-D to stop)\n");

        char* input = fgets(sendBuff, BUFFER_SIZE, stdin);

        if (input == NULL) {
            printf("Disconnecting from server\n");

            close(sock);
            exit(0);
            //return 0;
        }

        writen(sock, input, strlen(input));
        
        //readline(sock, readBuff, BUFFER_SIZE);
        readn(sock, readBuff, strlen(input));
        //read(sock, buffer, BUFFER_SIZE);

        printf("Message from server: %s\n", readBuff);
    }

    return 0;
}
