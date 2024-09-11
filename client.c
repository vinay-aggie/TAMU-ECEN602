#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 1024

ssize_t writen(int fd, const void *vptr, size_t n) {
    size_t nleft = n;
    ssize_t nwritten;
    const char *ptr = vptr;

    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0; // Restart write if interrupted
            else
                return -1; // Error
        }

        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

ssize_t readline(int fd, void *vptr, size_t maxlen) {
    size_t n;
    ssize_t rc;
    char c, *ptr;
    ptr = vptr;

    for (n = 1; n < maxlen; n++) {
        if ((rc = read(fd, &c, 1)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;
        } else if (rc == 0) {
            if (n == 1)
                return 0; // EOF, no data read
            else
                break;
        } else {
            if (errno == EINTR)
                continue;
            return -1; // Error
        }
    }
    *ptr = 0;
    return n;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s IPAddr Port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[MAXLINE];
    ssize_t n;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("inet_pton error");
        exit(EXIT_FAILURE);
    }

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect error");
        exit(EXIT_FAILURE);
    }

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (writen(client_socket, buffer, strlen(buffer)) < 0) {
            perror("write error");
            exit(EXIT_FAILURE);
        }

        if ((n = readline(client_socket, buffer, sizeof(buffer))) < 0) {
            perror("read error");
            exit(EXIT_FAILURE);
        }

        if (n == 0) {
            printf("Server closed connection\n");
            break;
        }

        fputs(buffer, stdout);
    }

    close(client_socket);
    return 0;
}
