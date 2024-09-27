
#ifndef SBCP_PROTOCOL_H
#define SBCP_PROTOCOL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "socketUtils.h"

struct sbcpAttribute
{
    uint16_t type;
    uint16_t length;
    char payload[BUFFER_SIZE];
};

struct sbcpPacket
{
    uint16_t version;
    uint16_t type;
    uint16_t length;
    struct sbcpAttribute *attr;
};

#define MAX_CLIENT_COUNT 3

static char usernames[MAX_CLIENT_COUNT][16] = {0};

static int currentClientCount  = 0;

int checkIfUserExists(char *buf) 
{
    for (int i = 0; i < MAX_CLIENT_COUNT; i++) 
    { 
        if (strcmp(buf, usernames[i]) == 0) 
        {
            printf("Username already exists! Take another one!\n"); 
            return 1;
        }
    }
    return 0; 
}

void addUserToDB(char *buf)
{
    for (int i = 0; i < MAX_CLIENT_COUNT; i++) 
    { 
        if (strcmp(usernames[i], "") == 0)
        {
            strcpy(usernames[i], buf);
        }
    }
}


int breakAttributesAndDetermineAction(char *readBuf, char *writeBuf, uint16_t totalSize, uint16_t version, uint16_t type, uint16_t sbcpRemLength)
{
    char sbcpAttributes[BUFFER_SIZE];
    memcpy(sbcpAttributes, readBuf, totalSize);

    int offset = 0;
    int counter = 0;
    char sbcpPacket[2][BUFFER_SIZE];
    int offsets[2] = {0,0};
    memset(sbcpPacket, 0, totalSize);
    while (offset < sbcpRemLength)
    {
        char temp[BUFFER_SIZE];
        printf("Printing packet!\n");
        uint16_t attrType;
        uint16_t attrLength;
        readAttribute(sbcpAttributes, temp, offset, &attrType, &attrLength);

        if (attrType == USERNAME) {
            memcpy(sbcpPacket[0] + offsets[0], temp, attrLength);
            offsets[0] += attrLength;
        } else if (attrType == MESSAGE) {
            memcpy(sbcpPacket[1] + offsets[1], temp, attrLength);
            offsets[1] += attrLength;
        }
        printf("type = (%d), length = (%d)\n", attrType, attrLength);
        sbcpPacket[counter][attrLength] = '\0';
        printf("Attribute {%i} has payload : (%s)\n", counter, sbcpPacket[counter]);
        offset = offset + HEADER_LENGTH + attrLength;
        counter++;
    }

    switch (type)
    {
        case JOIN:
            if (checkIfUserExists(sbcpPacket[0]) == 1)
            {
                printf("Please choose another user name. {%s} is already taken.\n", sbcpPacket[0]);
                return 1;
            }
            currentClientCount ++;
            if (currentClientCount > MAX_CLIENT_COUNT)
            {
                printf("Server has reached maximum capacity. You cannot join!\n");
                currentClientCount -- ;
                return 1;
            }

            printf("New user has joined the chat!\n");
            printf("User: %s\n", sbcpPacket[0]);
            addUserToDB(sbcpPacket[0]);
            // break here since we don't care even if there is a message.
            break;

        case SEND:
            printf("Forwarding message from User {%s} to all other users.\n", sbcpPacket[0]);

            addAttribute(writeBuf, 0, USERNAME, offsets[0], sbcpPacket[0]);
            addAttribute(writeBuf, HEADER_LENGTH + offsets[0], MESSAGE, offsets[1], sbcpPacket[1]);

            break;

        case FWD:
            printf("Message from %s: %s\n", sbcpPacket[0], sbcpPacket[1]);
        }

    return 0;
}

#endif /* SBCP_PROTOCOL_H */
