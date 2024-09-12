#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "socketUtils.h"

#define MAX_CONNECTIONS 100
#define BACKLOG 10

// Function declarations
void* handle_client(void* client_socket);
void sigchld_handler(int s);

// Main server function
int main(int argc, char *argv[])
{
    // Argument validation
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *portToConnect = argv[1];
    int serverSocketFd;
    struct sockaddr_in serverAddr;
    
    // Socket creation
    serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketFd == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully.\n");

    // Set socket options (SO_REUSEADDR) to reuse the socket immediately after closing
    int opt = 1;
    if (setsockopt(serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt failed");
        close(serverSocketFd);
        exit(EXIT_FAILURE);
    }

    // Initialize server address struct
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(portToConnect));
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(serverSocketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Bind failed");
        close(serverSocketFd);
        exit(EXIT_FAILURE);
    }
    printf("Socket binding successful.\n");

    // Set up signal handler to reap zombie processes
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction failed");
        close(serverSocketFd);
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(serverSocketFd, BACKLOG) == -1)
    {
        perror("Listen failed");
        close(serverSocketFd);
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %s...\n", portToConnect);

    // Main loop to accept and handle connections
    while (1)
    {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocketFd = accept(serverSocketFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocketFd == -1)
        {
            perror("Failed to accept connection");
            continue;
        }

        printf("New connection accepted.\n");

        // Handle each client in a new thread
        pthread_t thread_id;
        int *clientSock = malloc(sizeof(int));
        if (clientSock == NULL)
        {
            perror("Failed to allocate memory for client socket");
            close(clientSocketFd);
            continue;
        }
        *clientSock = clientSocketFd;
        if (pthread_create(&thread_id, NULL, handle_client, (void*)clientSock) != 0)
        {
            perror("Failed to create thread");
            free(clientSock);
            close(clientSocketFd);
        }
        else
        {
            pthread_detach(thread_id); // Detach thread to allow resources to be freed upon completion
        }
    }

    close(serverSocketFd);
    return 0;
}

// Function to handle communication with a client
void* handle_client(void* client_socket)
{
    int clientSock = *(int*)client_socket;
    free(client_socket); // Free allocated memory

    char buffer[BUFFER_SIZE];
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        ssize_t recBytes = read(clientSock, buffer, sizeof(buffer) - 1);
        if (recBytes == -1)
        {
            perror("Failed to read from socket");
            break;
        }
        if (recBytes == 0)
        {
            printf("Client disconnected.\n");
            break;
        }

        buffer[recBytes] = '\0'; // Ensure null termination
        printf("Message from client: %s\n", buffer);

        // Echo the message back to the client
        if (writen(clientSock, buffer, strlen(buffer)) == -1)
        {
            perror("Failed to write to socket");
            break;
        }
    }

    close(clientSock);
    return NULL;
}

// SIGCHLD handler to prevent zombie processes
void sigchld_handler(int s)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}
