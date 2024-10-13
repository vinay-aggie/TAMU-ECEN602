#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// const definitions.
#define BUFFER_SIZE 516
#define DATA_SIZE 512


// TFTP protocol.
#define  TFTP_RRQ    1
#define  TFTP_WRQ    2
#define  TFTP_DATA   3
#define  TFTP_ACK    4
#define  TFTP_ERROR  5



#define ASCII_MODE  1
#define BINARY_MODE 2


#define ERR_NOT_DEFINED      0
#define ERR_FILE_NOT_FOUND   1


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


void sendErrorMessage(unsigned int errorCode, char *errMsg, int sockfd, struct sockaddr_in *clientAddr, socklen_t clientAddrLen)
{
    char errBuf[BUFFER_SIZE];

    errBuf[0] = 0;
    errBuf[1] = 5;
    errBuf[2] = 0;
    errBuf[3] = 4;
    //errBuf[2] = (errorCode >> 8) & 0xFF;
    //errBuf[3] = errorCode & 0xFF;

    memcpy(errBuf + 4, errMsg, strlen(errMsg) + 1);
    size_t errBufLen = 2 + 2 + strlen(errBuf) + 1;

    if (sendto(sockfd, errBuf, errBufLen, 0, (struct sockaddr *)clientAddr, clientAddrLen) < 0)
    {
        printf("Some error handling for sendto!\n");
        return;
    }
}


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
    if (strcasecmp(modeOfOperation, "netascii"))
    {
        filePtr = fopen(fileName, "r");
    }
    else if (strcasecmp(modeOfOperation, "octet"))
    {
        filePtr = fopen(fileName, "rb");
    }
    else
    {
        printf("Unknown mode in RRQ packet!\n");
        char *msg = "Undefined mode of operation";
        sendErrorMessage(ERR_NOT_DEFINED, msg, origSockFd, clientAddr, clientAddrLen);
        return;
    }

    if (filePtr == NULL)
    {
        printf("File not found in server! Error = (%s)\n", strerror(errno));
        sendErrorMessage(ERR_FILE_NOT_FOUND, strerror(errno), origSockFd, clientAddr, clientAddrLen);
        return;
    }

    int packetNum = 1;
    char ackBuf[BUFFER_SIZE];
    while (1)
    {
        memset(sendBuf, 0 , BUFFER_SIZE);
        sendBuf[0] = 0;
        sendBuf[1] = 3;
        sendBuf[2] = (packetNum >> 8) & 0xFF;
        sendBuf[3] = packetNum & 0xFF;

        ssize_t bytesRead = fread(sendBuf + 4, 1, DATA_SIZE, filePtr);

        if (sendto(newSockFd, sendBuf, bytesRead + 4, 0, (struct sockaddr *)clientAddr, clientAddrLen) < 0)
        {
            printf("Some error handling for sendto!\n");
            break;
        }

        /*
        int timeouts = 1;
        while (timeouts <= 10)
        {
        }*/

        // need to implement as a loop of 10 as mentioned in the problem statement.
        if (setTimeout(newSockFd, 1) == -1)
        {
            printf("Error in timeout. Returning!\n");
            break;
        }

        if (recvfrom(newSockFd, ackBuf, BUFFER_SIZE, 0, (struct sockaddr *)clientAddr, &clientAddrLen) < 0)
        {
            printf("Some error hanndling for recv!\n");
            break;
        }

        packetNum++;
    }

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
        printf("File Name is: (%s)\n", fileName);
        size_t fileLen = strlen(fileName);
        printf("File length is: (%zu)\n", fileLen);
        char *mode = recBuf + 2 + (fileLen + 1);
        printf("Mode is: {%s}\n", mode);

        printf("Handling Read Request!\n");
        handleReadRequest(serverSocketFd, fileName, mode, &clientAddr, clientAddrLen);
    }

    return 0;
}