#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

#include "socketUtils.h"

int main (int argc, char *argv[])
{
    //freopen("/dev/null", "w", stdout);
    char storageBuf[BUFFER_SIZE];

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

    int serverSocketFd;
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

    int MaxFd;
    fd_set readFds;
    fd_set masterListFds;
    FD_ZERO(&readFds);
    FD_ZERO(&masterListFds);

    FD_SET(serverSocketFd, &masterListFds);
    MaxFd = serverSocketFd;

    while (1)
    {
        // Take a copy of the master list of file descriptors.
        // This is because the file descriptors passed to select is cleared
        // after processing.
        readFds = masterListFds;
        printf("MAX File Descriptor is {%d}\n", MaxFd);

        int retVal = select(MaxFd + 1, &readFds, NULL, NULL, NULL);
        if (retVal == -1)
        {
            printf("Failed to multiplex on the Client desciptors! retVal = (%d) Error = {%s}\n",
                    retVal, strerror(errno));
            exit(EXIT_FAILURE);
        }
        printf("We got an alert on a monitored descriptor\n");

        for (int descriptor = 0; descriptor <= MaxFd; descriptor++)
        {
            if (FD_ISSET(descriptor, &readFds) == 0)
            {
                continue;
            }

            printf("We got an alert on descriptor = (%d)\n", descriptor);
            // We have a File descriptor ready for reading.
            if (descriptor == serverSocketFd)
            {
                printf("Got a new connection!\n");
                socklen_t clientAddrLen = sizeof(clientAddr);
                int clientSocketFd = accept(serverSocketFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
                if (clientSocketFd == -1)
                {
                    printf("Failed to establish a connection with the client! clientSocketFd = [%d] Error = {%s}\n",
                           clientSocketFd, strerror(errno));
                    continue;
                }

                printf("Successfully established a connection with the client!\n");
                printf("Waiting for data...\n");

                // Add new client FD to the master list.
                FD_SET(clientSocketFd, &masterListFds);
                if (clientSocketFd > MaxFd)
                {
                    // Set new maximum based on the above
                    // condition.
                    MaxFd = clientSocketFd;
                }

                continue;
            }

            printf("Received data!\n");
            memset(storageBuf, 0, sizeof(storageBuf));
            //int recBytes = read(descriptor, storageBuf, BUFFER_SIZE);
            int retVal = receiveMessage(descriptor, storageBuf, BUFFER_SIZE);
            if (retVal == -1)
            {
                printf("Failed to read data from socket! Error = {%s}\n", strerror(errno));
                close(descriptor);
                FD_CLR(descriptor, &masterListFds);
                continue;
            }
            // Terminate when an end-of-file is received indicated by 0 bytes received.
            if (retVal == 0)
            {
                printf("Received EOF from client. Terminating connection!\n");
                close(descriptor);
                FD_CLR(descriptor, &masterListFds);
                continue;
            }
            printf("Received data!\n");
            printf("Message from client: %s \n", storageBuf);

            // send to all other connections!
            for (int allConnections = 0; allConnections <= MaxFd; allConnections++)
            {
                if (FD_ISSET(allConnections, &masterListFds) == 0)
                {
                    // Don't send data to non-existant file descriptors.
                    continue;
                }

                // Don't send data to listening socket as well as the client
                // from which we received data.
                if ((allConnections == serverSocketFd) || (allConnections == descriptor))
                {
                    continue;
                }

                printf("Sending data to client (%d)\n", allConnections);

                //int retVal = send(allConnections, storageBuf, strlen(storageBuf), 0);
                int sendData = sendMessage(allConnections, 3, 3, strlen(storageBuf), storageBuf);

                if (retVal == -1)
                {
                    printf("Failed to establish a connection with the client! clientSocketFd = [%d] Error = {%s}\n",
                            allConnections, strerror(errno));
                }
            }
            printf("Finished sending data\n");
        }

        printf("Finished iterating through descriptors\n");
    }

    return 0;
}