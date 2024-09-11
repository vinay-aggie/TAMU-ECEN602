#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 1024

void handle_client(int client_socket) {
    char buffer[MAXLINE];
    ssize_t n;

    while ((n = read(client_socket, buffer, sizeof(buffer))) > 0) {
        if (write(client_socket, buffer, n) < 0) {
            perror("write error");
            close(client_socket);
            exit(EXIT_FAILURE);
        }
    }
    
    if (n < 0) {
        perror("read error");
    }

    close(client_socket);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s Port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pid_t child_pid;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) < 0) {
        perror("listen error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("accept error");
            continue;
        }

        if ((child_pid = fork()) == 0) {
            close(server_socket);
            handle_client(client_socket);
        } else if (child_pid < 0) {
            perror("fork error");
            close(client_socket);
        }

        close(client_socket);
        while (waitpid(-1, NULL, WNOHANG) > 0) {
            // Clean up finished child processes
        }
    }

    close(server_socket);
    return 0;
}
