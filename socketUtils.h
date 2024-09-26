// Header file.

#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <stdio.h>
#include <stdint.h>

#define MAX_USERNAME 128
#define BUFFER_SIZE 65536

// protocol version
#define VERSION 3

// header types
#define JOIN 2
#define SEND 4
#define FWD 3

// header length
#define HEADER_LENGTH 4

#define MAX_CONNECTIONS 100

#define USERNAME 2
#define MESSAGE 4
#define REASON 1
#define CLIENT_COUNT 3

void addAttribute(char* buff, uint16_t offset, uint16_t type, uint16_t length, const char* payload);

int sendMessage(int socket, uint16_t version, uint16_t type, uint16_t length, const char *payload);

int receiveMessage(int fd, char *buf, int buf_size);

void readAttribute(char *readBuff, char* writeBuff, uint16_t offset, uint16_t* type, uint16_t* length);

#endif /* SOCKET_UTILS_H */