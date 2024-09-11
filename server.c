#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

//#include "socketUtils.h"
// const definitions.
#define BUFFER_SIZE 512
#define MAX_CONNECTIONS 10

// Method to write 'n' bytes to a buffer when provided with a
// socket descriptor and a pointer to the user level buffer.
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

int main (int argc, char *argv[])
{
    if (argc > 2)
    {
        printf("Only 2 arguments allowed! Please try again...\n");
        exit(EXIT_FAILURE);
    }
    else if (argc == 1)
    {
        printf("Too few arguments! Please enter the port number to connect to.\n");
        exit(EXIT_FAILURE);
    }

    char *portToConnect = argv[1];

    int serverSocketFd, clientSocketFd;
    struct sockaddr_in serverAddr, clientAddr;

    serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketFd == -1)
    {
        printf("Failed to create socket! serverSocketFd = [%d] Error = {%s}\n", serverSocketFd, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully.\n");

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(portToConnect));
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    int retVal;
    int retVal = bind(serverSocketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (retVal == -1)
    {
        printf("Failed to bind socket! retVal = [%d] Error = {%s}\n", retVal, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("Socket binding successfull.\n");

    retVal = listen(serverSocketFd, MAX_CONNECTIONS);
    if (retVal == -1)
    {
        printf("Failed to listen for incoming connections! retVal = [%d] Error = {%s}\n", retVal, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("Listening for incoming connections...\n");

    char storageBuf[BUFFER_SIZE];
    while(1)
    {
        socklen_t clientAddrLen = sizeof(clientAddr);
        clientSocketFd = accept(serverSocketFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocketFd == -1)
        {
            printf("Failed to establish a connection with the client! clientSocketFd = [%d] Error = {%s}\n", clientSocketFd, strerror(errno));
            exit(EXIT_FAILURE);
        }

        printf("Successfully established a connection with the client!\n");
        printf("Waiting for data...\n");

        pid_t forkRet = fork();
        if (forkRet < 0)
        {
            printf("Failed to fork new client handler. Exiting! Error = {%s}\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (forkRet > 0)
        {
            // parent process.
            close(clientSocketFd);
            continue;
        }
        
        // child process
        close(serverSocketFd);

        while (1)
        {
            memset(storageBuf, 0, sizeof(storageBuf));
            int recBytes = read(clientSocketFd, storageBuf, BUFFER_SIZE);
            if (recBytes == -1)
            {
                printf("Failed to read data from socket! Error = {%s}\n", strerror(errno));
                close(clientSocketFd);
                exit(EXIT_FAILURE);
            }
            if (recBytes == 0)
            {
                printf("Received EOF from client. Terminating connection!\n");
                close(clientSocketFd);
                exit(EXIT_FAILURE);
            }
            storageBuf[recBytes] = '\0';
            printf("Message from client: %s \n", storageBuf);

            writen(clientSocketFd, storageBuf, strlen(storageBuf));
            //send(clientSocketFd, storageBuf, strlen(storageBuf), 0);
        }

    }

    close(clientSocketFd);
    close(serverSocketFd);

    return 0;
}
