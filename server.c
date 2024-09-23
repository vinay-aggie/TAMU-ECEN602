#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

// const definitions.
#define MAX_CONNECTIONS 100

#define BUFFER_SIZE 1024

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
        close(serverSocketFd);
        exit(EXIT_FAILURE);
    }
    printf("Socket binding successfull.\n");

    // Listen for connections. We can handle upto 100 connections simlutaneously.
    // 100 should be good for now but change this value as and recompile if required.
    retVal = listen(serverSocketFd, MAX_CONNECTIONS);
    // Exit if listening failed.
    if (retVal == -1)
    {
        printf("Failed to listen for incoming connections! retVal = [%d] Error = {%s}\n", retVal, strerror(errno));
        close(serverSocketFd);
        exit(EXIT_FAILURE);
    }
    printf("Listening for incoming connections...\n");

    char storageBuf[BUFFER_SIZE];
    int fdMax;
    fd_set read_fds;
    fd_set master;
    FD_ZERO(&read_fds);
    FD_ZERO(&master);
    FD_SET(serverSocketFd, &master);

    fdMax = serverSocketFd;
    while (1)
    {
        read_fds = master;
        if (select(fdMax + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("select");
            exit (EXIT_FAILURE);
        }

        for (int i = 0; i <= fdMax; i++)
        {
            if (FD_ISSET(i, &read_fds))
            {
                if (i == serverSocketFd)
                {
                    socklen_t clientAddrLen = sizeof(clientAddr);
                    clientSocketFd = accept(serverSocketFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
                    if (clientSocketFd == -1)
                    {
                        printf("Failed to establish a connection with the client! clientSocketFd = [%d] Error = {%s}\n", clientSocketFd, strerror(errno));
                    }
                    else
                    {
                        FD_SET(clientSocketFd, &master);
                        if (clientSocketFd > fdMax)
                        {
                            fdMax = clientSocketFd;
                        }
                        printf("Successfully established a connection with the client!\n");
                        printf("Waiting for data...\n");
                    }
                }
                else
                {
                    memset(storageBuf, 0, sizeof(storageBuf));
                    int recBytes = read(clientSocketFd, storageBuf, BUFFER_SIZE);
                    if (recBytes == -1)
                    {
                        printf("Failed to read data from socket! Error = {%s}\n", strerror(errno));
                        close(clientSocketFd);
                        FD_CLR(i, &master);
                        continue;
                    }
                    // Terminate when an end-of-file is received indicated by 0 bytes received.
                    if (recBytes == 0)
                    {
                        printf("Received EOF from client. Terminating connection!\n");
                        close(clientSocketFd);
                        FD_CLR(i, &master);
                        continue;
                    }
                    printf("Received data!\n");
                    printf("Message from client: %s \n", storageBuf);

                    // send to all other connections!
                    for (int j = 0; j <= fdMax; j++)
                    {
                        printf("Sending data to client (%d)\n", j);
                        if (FD_ISSET(j, &master))
                        {
                            if ((j != serverSocketFd) && (j != i))
                            {
                                if (send(j, storageBuf, strlen(storageBuf), 0) == -1)
                                {
                                    perror("sending error!\n");
                                }
                            }
                        }
                    }
                }
            }
        }


    }

    return 0;
}