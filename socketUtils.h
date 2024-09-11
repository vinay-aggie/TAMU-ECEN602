// Header file.

#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <stdio.h>

#define BUFFER_SIZE 512

int writen(int clientFd, const char *ptrBuf, size_t bytesToWrite);

ssize_t readline(int fd, char *ptr, size_t maxlen);

#endif /* SOCKET_UTILS_H */
