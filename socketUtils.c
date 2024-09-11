#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>

#include "socketUtils.h"

// Method to write 'n' bytes to a buffer when provided with a
// socket descriptor and a pointer to the user level buffer.
int writen(int clientFd, const char *ptrBuf, size_t bytesToWrite)
{
    size_t bytesLeft;
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
    return (int)bytesWritten;
}

ssize_t readline(int fd, char *ptr, size_t maxlen) {
    ssize_t n, rc;
    char c;

    for (n = 1; n < maxlen; n++) {
        again:
            if ((rc = read(fd, &c, 1)) == 1) {
                *ptr++ = c;
                if (c == '\n')
                    break;
            } else if (rc == 0) {
                *ptr = 0;
                return (n - 1);
            } else {
                if (errno == EINTR) {
                    goto again;
                }
                return -1;
            }
    }

    *ptr = 0;
    return n;
}
