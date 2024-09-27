#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define PORT 12345

// Send a message to all clients except the sender
void broadcast_message(int sender_fd, int *client_fds, int num_clients, char *message, int message_len) {
    for (int i = 0; i < num_clients; i++) {
        int client_fd = client_fds[i];
        if (client_fd != sender_fd && client_fd != 0) {
            if (send(client_fd, message, message_len, 0) < 0) {
                perror("Failed to send message");
            }
        }
    }
}

int main() {
    int server_fd, client_fd, max_fd, activity, new_socket, valread;
    int client_fds[MAX_CLIENTS] = {0};  // Store client socket descriptors
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    fd_set readfds;

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address (IPv4)
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d\n", PORT);

    while (1) {
        // Clear the socket set and add the server socket
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_fd = server_fd;

        // Add client sockets to the set
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_fds[i] > 0) FD_SET(client_fds[i], &readfds);
            if (client_fds[i] > max_fd) max_fd = client_fds[i];
        }

        // Wait for activity on one of the sockets
        activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("Select error");
        }

        // Handle new connections
        if (FD_ISSET(server_fd, &readfds)) {
            new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
            if (new_socket < 0) {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }

            printf("New connection: socket fd %d, ip %s, port %d\n", new_socket,
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            // Add new client socket to the array
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_fds[i] == 0) {
                    client_fds[i] = new_socket;
                    printf("Added client to slot %d\n", i);
                    break;
                }
            }
        }

        // Handle IO for each client
        for (int i = 0; i < MAX_CLIENTS; i++) {
            client_fd = client_fds[i];

            if (FD_ISSET(client_fd, &readfds)) {
                // Check if the client is closing
                if ((valread = read(client_fd, buffer, BUFFER_SIZE)) == 0) {
                    // Client disconnected
                    getpeername(client_fd, (struct sockaddr *)&client_addr, &addr_len);
                    printf("Client disconnected: ip %s, port %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    close(client_fd);
                    client_fds[i] = 0;
                } else {
                    // Broadcast message to other clients
                    buffer[valread] = '\0';
                    broadcast_message(client_fd, client_fds, MAX_CLIENTS, buffer, valread);
                }
            }
        }
    }

    return 0;
}
