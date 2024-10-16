#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>

// const definitions.
#define BUFFER_SIZE 516
#define DATA_SIZE 512


// TFTP protocol.
#define  TFTP_RRQ    1
#define  TFTP_WRQ    2
#define  TFTP_DATA   3
#define  TFTP_ACK    4
#define  TFTP_ERROR  5



#define ASCII_MODE    1
#define BINARY_MODE   2
#define UNKNOWN_MODE  3


#define ERR_NOT_DEFINED      0
#define ERR_FILE_NOT_FOUND   1


#define MAX_RETRIES    10

// ref: Beej's guide.
void reapChildZombies(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    int pid;
    while ((pid =  waitpid(-1, NULL, WNOHANG)) > 0)
    {
        printf("Reaped client handler: (%d)\n", pid);
    }

    errno = saved_errno;
}


// Set a timeout == timeInSec
// If timouet expires without fileDesc set, then
// return is 0, else positive.
// Return -1 if select fails.
int setTimeout(int fileDesc, int timeInSec)
{
    fd_set rSet;
    struct timeval tv;

    FD_ZERO(&rSet);
    FD_SET(fileDesc, &rSet);

    tv.tv_sec = timeInSec;
    tv.tv_usec = 0;

    return (select(fileDesc + 1, &rSet, NULL, NULL, &tv));
}


// Method to pack and send an error message.
void sendErrorMessage(unsigned int errorCode, char *errMsg, int sockfd, struct sockaddr_in *clientAddr, socklen_t clientAddrLen)
{
    char errBuf[BUFFER_SIZE];

    errBuf[0] = 0;
    errBuf[1] = TFTP_ERROR;
    errBuf[2] = (errorCode >> 8) & 0xFF;
    errBuf[3] = errorCode & 0xFF;

    memcpy(errBuf + 4, errMsg, strlen(errMsg) + 1);

    size_t errBufLen = 2 + 2 + strlen(errMsg) + 1;

    if (sendto(sockfd, errBuf, errBufLen, 0, (struct sockaddr *)clientAddr, clientAddrLen) < 0)
    {
        printf("Some error handling for sendto!\n");
        return;
    }
}


// Method to handle a RRQ from the client.
void handleReadRequest(int origSockFd, char *fileName, char *modeOfOperation, struct sockaddr_in *clientAddr, socklen_t clientAddrLen)
{
    struct sockaddr_in ephemeralAddr;
    char sendBuf[BUFFER_SIZE];

    ephemeralAddr.sin_family = AF_INET;
    ephemeralAddr.sin_port = htons(0);
    ephemeralAddr.sin_addr.s_addr = INADDR_ANY;

    int newSockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (newSockFd < 0)
    {
        printf("Failed to create socket! newSockFd = [%d] Error = {%s}\n", newSockFd, strerror(errno));
        exit(EXIT_FAILURE);
    }

    int retVal = bind(newSockFd, (struct sockaddr*)&ephemeralAddr, sizeof(ephemeralAddr));
    if (retVal < 0)
    {
        printf("Failed to bind socket! retVal = [%d] Error = {%s}\n", retVal, strerror(errno));
        close(newSockFd);
        exit(EXIT_FAILURE);
    }

    FILE *filePtr;
    int modeOp;
    if (strcasecmp(modeOfOperation, "netascii") == 0)
    {
        modeOp = ASCII_MODE;
        filePtr = fopen(fileName, "r");
    }
    else if (strcasecmp(modeOfOperation, "octet") == 0)
    {
        modeOp = BINARY_MODE;
        filePtr = fopen(fileName, "rb");
    }
    else
    {
        modeOp = UNKNOWN_MODE;
        printf("Unknown mode in RRQ packet!\n");
        char *errMsg = "Undefined mode of operation";
        sendErrorMessage(ERR_NOT_DEFINED, errMsg, origSockFd, clientAddr, clientAddrLen);
        return;
    }

    if (filePtr == NULL)
    {
        char *errMsg = strerror(errno);
        printf("File not found in server! Error = (%s)\n", errMsg);
        sendErrorMessage(ERR_FILE_NOT_FOUND, errMsg, origSockFd, clientAddr, clientAddrLen);
        return;
    }

    int packetNum = 1;
    char ackBuf[BUFFER_SIZE];
    char nextChar = -1;
    while (1)
    {
        printf("Block Number is: (%d)\n", packetNum);
        // packet num wrap around
        if (packetNum > 65535)
        {
            packetNum = 0;
        } 
        memset(sendBuf, 0 , BUFFER_SIZE);
        sendBuf[0] = 0;
        sendBuf[1] = TFTP_DATA;
        sendBuf[2] = (packetNum >> 8) & 0xFF;
        sendBuf[3] = packetNum & 0xFF;

        char *ptr = sendBuf + 4;
        ssize_t bytesRead = 0;
        if (modeOp == ASCII_MODE)
        {
            ssize_t count = -1;
            // Ref: Unix network programming.
            for (count = 0; count < DATA_SIZE; count ++)
            {
                if (nextChar >= 0)
                {
                    *ptr++ = nextChar;
                    nextChar = -1;
                    continue;
                }

                char currentChar = fgetc(filePtr);

                if (currentChar == EOF)
                {
                    printf("Reached EOF! Finished reading file.\n");
                    bytesRead = count;
                    break;
                }
                else if (currentChar == '\n')
                {
                    currentChar = '\r';
                    nextChar = '\n';
                }
                else if (currentChar == '\r')
                {
                    nextChar = '\0';
                }
                else
                {
                    nextChar = -1;
                }

                *ptr++ = currentChar;

            }
            bytesRead = count;
        }
        else if (modeOp == BINARY_MODE)
        {
            bytesRead = fread(sendBuf + 4, 1, DATA_SIZE, filePtr);
        }

        printf("Bytes read from file = (%zu)\n", bytesRead);

        int retry = 0;
        while (retry < MAX_RETRIES)
        {
            if (sendto(newSockFd, sendBuf, bytesRead + 4, 0, (struct sockaddr *)clientAddr, clientAddrLen) < 0)
            {
                printf("Failed to send TFTP data packet! Error = {%s}\n", strerror(errno));
                retry ++;
                continue;
            }

            int ret = setTimeout(newSockFd, 1);

            if (ret <= 0)
            {
                printf("Timeout occurred. Sending data packet (%d) again! Timeout number: (%d)\n", packetNum, retry + 1);
                retry ++;
                continue;
            }
            if (recvfrom(newSockFd, ackBuf, BUFFER_SIZE, 0, (struct sockaddr *)clientAddr, &clientAddrLen) < 0)
            {
                printf("Failed to receive TFTP data packet! Error = {%s}\n", strerror(errno));
                retry ++;
                continue;
            }

            unsigned ackOp = ackBuf[1];
            unsigned int ackNum = ((ackBuf[2] << 8) | ackBuf[3]);

            printf("ACK opcode = (%d)\n", ackOp);
            printf("ACK Number = (%d)\n", ackNum);

            if ((ackOp != TFTP_ACK))
            {
                printf("Incorred ACK received! Send packet again\n");
                retry ++;
                continue;
            }

            if (bytesRead < 512)
            {
                printf("Received last ACK. Transmission complete!\n");
                break;
            }

            break;
        }

        if (retry ==  MAX_RETRIES)
        {
            printf("10 timeouts occurred. Client not present. Disconnecting!\n");
            fclose(filePtr);
            return;
        }

        packetNum++;
    }

    printf("Done sending file!\n");

    fclose(filePtr);
    return;
}




int main (int argc, char *argv[])
{
    // Some pre-flight checks.
    if (argc > 2)
    {
        printf("Only 2 arguments allowed! Please try again...\n");
        exit(EXIT_FAILURE);
    }
    else if (argc == 1)
    {
        printf("Too few arguments! Please enter the port number to connect to.\n");
        exit(EXIT_FAILURE);
    }

    // Fetch the port as argument.
    char *portToConnect = argv[1];

    int serverSocketFd;
    struct sockaddr_in serverAddr, clientAddr;

    // Create a socket descriptor for an IPV4 address and handling TCP connections.
    serverSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocketFd == -1)
    {
        printf("Failed to create socket! serverSocketFd = [%d] Error = {%s}\n", serverSocketFd, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully.\n");

    // Initialize the server struct with the necessary data.
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(portToConnect));
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind to the socket descriptor created earlier.
    int retVal = bind(serverSocketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    // Exit if binding failed.
    if (retVal == -1)
    {
        printf("Failed to bind socket! retVal = [%d] Error = {%s}\n", retVal, strerror(errno));
        close(serverSocketFd);
        exit(EXIT_FAILURE);
    }

    // register signal handler to reap zombies.
    struct sigaction sa;
    sa.sa_handler = reapChildZombies; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        printf("Failed to register signal handler to reap child processes! Error = {%s}\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Socket binding successfull.\n");
    printf("TFTP server listening for incoming connections...\n");

    while (1)
    {
        char recBuf[BUFFER_SIZE];
        memset(recBuf, 0, BUFFER_SIZE);
        socklen_t clientAddrLen = sizeof(clientAddr);
        // Receive a message from the client
        ssize_t recBytes = recvfrom(serverSocketFd, recBuf, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (recBytes < 0)
        { 
            printf("Failed to read data from socket! Error = {%s}\n", strerror(errno));
            continue;
        }
            
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        printf("Established connecttion with client having IP: (%s) \n", clientIP);
        printf("Received data from client! Reading...\n");

        // After receiving data, we fork the process and pass over the responsibility
        // to the child.
        pid_t forkRet = fork();
        if (forkRet < 0)
        {
            printf("Failed to fork client handler. Exiting! Error = {%s}\n", strerror(errno));
            // cleanup all descriptors and exit
            close(serverSocketFd);
            exit(EXIT_FAILURE);
        }

        if (forkRet > 0)
        {
            // Parent process.
            printf("Parent\n");
            continue;
        }

        // Handle client data.
        // child.
        unsigned char opCode = recBuf[1];
        if (opCode != TFTP_RRQ)
        {
            printf("The request is not TFTP Read Request. Cannot process! Exiting\n");
            exit(EXIT_FAILURE);
        }
        char *fileName =  recBuf + 2;
        printf("File to retrieve: {%s}\n", fileName);
        size_t fileLen = strlen(fileName);
        char *mode = recBuf + 2 + (fileLen + 1);
        printf("Mode of Operation: {%s}\n", mode);

        printf("Handling Read Request!\n");
        handleReadRequest(serverSocketFd, fileName, mode, &clientAddr, clientAddrLen);
        exit(0);
    }

    return 0;
}
