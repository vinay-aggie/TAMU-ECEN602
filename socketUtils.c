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

    printf("Howdy\n");
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

int receiveMessage(int fd, char *buf, int buf_size)
{
    char tempBuff[BUFFER_SIZE];

    int recBytes = read(fd, tempBuff, strlen(tempBuff));
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

    printf("Length of received message = (%d)\n", buf_size);

    uint16_t version = ((tempBuff[0] << 1) | ((tempBuff[1] & 0x80) >> 7));
    uint16_t type = tempBuff[1] & 0x7F;
    uint16_t length = (tempBuff[2] << 8 | tempBuff[3]);

    printf("Version = (%d), Type = (%d), Length = (%d)\n", version, type, length);

    for (int i = 0; i < length; i++) {
        buf[i] = tempBuff[i + HEADER_LENGTH];
    }
    
    //exit (EXIT_FAILURE);

    return recBytes;
}


/*******************************************************
 * writen function:
 *      Method to write 'n' bytes to a buffer when provided with a socket descriptor and a pointer to the user level buffer.
 * 
 * Inputs:
 *      int clientFd: socket connection between server and client
 *      const char *ptrBuf: pointer to the buffer containing text to be sent through the socket
 *      size_t bytesToWrite: size of the packet to send
 * 
 * Outputs:
 *      int: Total number of bytes written to socket
 ******************************************************/
ssize_t writen(int clientFd, const char *ptrBuf, size_t bytesToWrite)
{
    ssize_t bytesLeft;
    ssize_t bytesWritten;

    if (bytesToWrite % BUFFER_SIZE == 1) {
        bytesToWrite -= 1;
    }

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
    return bytesWritten;
}

/* ******************************************************
 * readline function:
 *      Reads data of fixed size coming through socket
 * 
 * Inputs:
 *      int fd: socket connection between server and client
 *      const char *ptrBuf: pointer to the buffer to be filled with text sent through the socket
 *      size_t bytesToRead: size of the packet that was sent
 * 
 * Outputs:
 *      int: Total number of bytes read from the socket
 ****************************************************** */
ssize_t readline(int fd, char *ptrBuff, size_t bytesToRead) {
    ssize_t bytesLeft;
    ssize_t bytesRead;

    bytesLeft = bytesToRead;
    while (bytesLeft > 0) {
        if ((bytesRead = read(fd, ptrBuff, bytesLeft)) < 0) {
            if (errno == EINTR) {
                bytesRead = 0;
            } else {
                return -1;
            }
        } else if (bytesRead == 0) {
            break;
        }

        bytesLeft -= bytesRead;
        ptrBuff += bytesRead;
    }

    return bytesToRead - bytesLeft;
}