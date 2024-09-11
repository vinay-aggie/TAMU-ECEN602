#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>

#include "socketUtils.h"

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
