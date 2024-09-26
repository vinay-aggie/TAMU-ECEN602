#include "socketUtils.h"

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>


void addAttribute(char* buff, uint16_t offset, uint16_t type, uint16_t length, const char* payload) {
    uint8_t attributeHeader[HEADER_LENGTH];

    attributeHeader[0] = (type >> 8) & 0xff;
    attributeHeader[1] = type & 0xff;
    attributeHeader[2] = (length >> 8) & 0xff;
    attributeHeader[3] = length & 0xff;

    if (offset + HEADER_LENGTH + length > BUFFER_SIZE) {
        printf("Buffer overflow\n");
        return;
    }

    memcpy(buff + offset, attributeHeader, HEADER_LENGTH);
    memcpy(buff + offset + HEADER_LENGTH, payload, length);
}

int sendMessage(int socket, uint16_t version, uint16_t type, uint16_t length, const char *payload) {
    uint8_t header[HEADER_LENGTH];

    header[0] = version >> 1;
    header[1] = (((version & 0x01) << 7) + (type & 0x7F));
    header[2] = length >> 8;
    header[3] = length & 0xFF;

    size_t packet_size = HEADER_LENGTH + length;

    unsigned char* message = malloc(packet_size);

    memcpy(message, header, HEADER_LENGTH);
    memcpy(message + HEADER_LENGTH, payload, length);

    ssize_t bytes_sent = send(socket, message, packet_size, 0);
    
    return bytes_sent;
}

int receiveMessage(int fd, char *buf, uint16_t *_version, uint16_t *_type, uint16_t *_length)
{
    char tempBuff[BUFFER_SIZE];

    int recBytes = read(fd, tempBuff, BUFFER_SIZE);
    if (recBytes == -1)
    {
        printf("Failed to read data from socket! Error = {%s}\n", strerror(errno));
        return -1;
    }
    // Terminate when an end-of-file is received indicated by 0 bytes received.
    if (recBytes == 0)
    {
        printf("Received EOF from client. Terminating connection!\n");
        return -1;
    }

    printf("Length of received message = (%d)\n", recBytes);

    uint16_t version = ((tempBuff[0] << 1) | ((tempBuff[1] & 0x80) >> 7));
    uint16_t type = tempBuff[1] & 0x7F;
    uint16_t length = (tempBuff[2] << 8 | tempBuff[3]);

    printf("Version = (%d), Type = (%d), Length = (%d)\n", version, type, length);

    for (int i = 0; i < length; i++) {
        buf[i] = tempBuff[i + HEADER_LENGTH];
    }

    *_version = version;
    *_type = type;
    *_length = length;

    return recBytes;
}

void readAttribute(char *readBuff, char* writeBuff, uint16_t offset, uint16_t *_type, uint16_t *_length)
{
    // Take in the buffer that holds the message data, a buffer to write to, and an offset
    // read the buffer, starting at offset, to find the header, determine the length, and fill the writeBuff with the data
    // return the length of the data, which should be held in writeBuff

    char header[HEADER_LENGTH];

    memcpy(header, readBuff + offset, HEADER_LENGTH);

    uint16_t type = (header[0] << 8) | (header[1] & 0xff);
    uint16_t length = (header[2] << 8) | (header[3] & 0xff);

    memcpy(writeBuff, readBuff + offset + HEADER_LENGTH, length);

    *_type = type;
    *_length = length;
}