
#ifndef SBCP_PROTOCOL_H
#define SBCP_PROTOCOL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "socketUtils.h"

/*
enum headerTypes
{
    Join = 2,
    Send = 4,
    Fwd = 3,
};

enum attributeTypes
{
    Username = 2,
    Message  = 4,
    Reason = 1,
    ClientCount  = 3,
};*/

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

char usernames[MAX_CONNECTIONS][16] = {0};

void breakAttributesAndDetermineAction(char *readBuf, char *writeBuf, uint16_t totalSize, uint16_t version, uint16_t type, uint16_t sbcpRemLength)
{
    char sbcpAttributes[BUFFER_SIZE];
    memcpy(sbcpAttributes, readBuf, totalSize);

    int offset = 0;
    int counter = 0;
    char sbcpPacket[2][BUFFER_SIZE];
    memset(sbcpPacket, 0 ,sizeof(sbcpPacket));
    while (offset < sbcpRemLength)
    {
        printf("Printing packet!\n");
        uint16_t attrType;
        uint16_t attrLength;
        readAttribute(sbcpAttributes, sbcpPacket[counter], offset, &attrType, &attrLength);
        printf("type = (%d), length = (%d)\n", attrType, attrLength);
        sbcpPacket[counter][attrLength] = '\0';
        printf("Attribute 1 has payload : (%s)\n", sbcpPacket[counter]);
        offset = offset + HEADER_LENGTH + attrLength;
        counter++;
    }

    switch (type)
    {
        case JOIN:
            // check if user exists.
            printf("New user has joined the chat!\n");
            printf("User: %s\n", sbcpPacket);
            // break here since we don't care even if there is a message.
            break;

        case SEND:
            printf("Forwarding message from User {%s} to all other users.\n");
            addAttribute(writeBuf, 0, USERNAME, strlen(sbcpPacket[0]), sbcpPacket[0]);
            addAttribute(writeBuf, HEADER_LENGTH + strlen(sbcpPacket[0]), MESSAGE, strlen(sbcpPacket[1]), sbcpPacket[1]);
            break;
        }
}
/*
struct sbcpUsernamePacket
{
    uint16_t type;

}

struct sbcpProtocol
{
    uint16_t version;
    uint16_t type;
    uint16_t length;
    struct sbcpAttributePacket[] payload;
};*/

#endif /* SBCP_PROTOCOL_H */
