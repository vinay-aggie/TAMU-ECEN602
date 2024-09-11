#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "socketUtils.h"

// const definitions.
#define MAX_CONNECTIONS 100

/*
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
} */

int main (int argc, char *argv[])
{
    // Some pre-flight checks.
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

    // Fetch the port as argument.
    char *portToConnect = argv[1];

    int serverSocketFd, clientSocketFd;
    struct sockaddr_in serverAddr, clientAddr;

    // Create a socket descriptor for an IPV4 address and handling TCP connections.
    serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketFd == -1)
    {
        printf("Failed to create socket! serverSocketFd = [%d] Error = {%s}\n", serverSocketFd, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully.\n");

    // Initialize the server struct with the necessary data.
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(portToConnect));
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind to the socket descriptor created earlier.
    int retVal = bind(serverSocketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    // Exit if binding failed.
    if (retVal == -1)
    {
        printf("Failed to bind socket! retVal = [%d] Error = {%s}\n", retVal, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("Socket binding successfull.\n");

    // Listen for connections. We can handle 100 connections simlutaneously.
    // 100 should be good for now but change this value as and recompile if required.
    retVal = listen(serverSocketFd, MAX_CONNECTIONS);
    // Exit if listening failed.
    if (retVal == -1)
    {
        printf("Failed to listen for incoming connections! retVal = [%d] Error = {%s}\n", retVal, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("Listening for incoming connections...\n");

    char storageBuf[BUFFER_SIZE];
    // Infinitely monitor for incoming client connections.
    while(1)
    {
        socklen_t clientAddrLen = sizeof(clientAddr);
        // 'accept' is blocking. If this line executes with a return of 0 then we got a client connection.
        clientSocketFd = accept(serverSocketFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocketFd == -1)
        {
            printf("Failed to establish a connection with the client! clientSocketFd = [%d] Error = {%s}\n", clientSocketFd, strerror(errno));
            close(serverSocketFd);
            exit(EXIT_FAILURE);
        }

        printf("Successfully established a connection with the client!\n");
        printf("Waiting for data...\n");

        // After accepting a connection, we fork the process and pass over the responsibility
        // of handling the connected client to the child process.
        // The parent is responsible for handling future incoming connection requests.
        pid_t forkRet = fork();
        // If forking fails, we terminate.
        if (forkRet < 0)
        {
            printf("Failed to fork new client handler. Exiting! Error = {%s}\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (forkRet > 0)
        {
            // Parent process.
            // Close the client file descriptor as the child already has the copy.
            close(clientSocketFd);
            continue;
        }

        // Child process.
        // Close the server file descriptor as the child does not require this for reading data.
        close(serverSocketFd);

        // Continuously monitor the client for data, unless we receive 'termination' signals.
        while (1)
        {
            // always set the buffer to 0 before beginning the read operation.
            memset(storageBuf, 0, sizeof(storageBuf));
            int recBytes = read(clientSocketFd, storageBuf, BUFFER_SIZE);
            // Failed to read data and hence exit.
            if (recBytes == -1)
            {
                printf("Failed to read data from socket! Error = {%s}\n", strerror(errno));
                close(clientSocketFd);
                exit(EXIT_FAILURE);
            }
            // Terminate when an end-of-file is received indicated by 0 bytes received.
            if (recBytes == 0)
            {
                printf("Received EOF from client. Terminating connection!\n");
                close(clientSocketFd);
                exit(EXIT_FAILURE);
            }
            // Null terminate and print the string.
            storageBuf[recBytes] = '\0';
            printf("Message from client: %s \n", storageBuf);

            writen(clientSocketFd, storageBuf, strlen(storageBuf));
        }

    }

    return 0;
}
