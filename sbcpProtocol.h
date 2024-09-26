
#ifndef SBCP_PROTOCOL_H
#define SBCP_PROTOCOL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "socketUtils.h"

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
};

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
    sbcpAttribute *attr;
};

char usernames[MAX_CONNECTIONS][16] = {0};

void breakAttributes(char *readBuf, uint16_t totalSize, uint16_t version, uint16_t type, uint16_t sbcpRemLength)
{
    char sbcpAttributes[BUFFER_SIZE];
    memcpy(sbcpAttributes, readBuf, totalSize);

    /*
    if ((enum headerTypes)type == headerTypes::Join)
    {
        printf("We got a new user!\n");
        uint16_t type;
        uint16_t length;
        char sbcpPacket[BUFFER_SIZE];
        readAttribute(sbcpAttributes, sbcpPacket, 0, &type, &length);

        printf("User join chat: %s", sbcpPacket)
    }*/

    int offset = 0;
    while (offset < sbcpRemLength)
    {
        printf("Printing packet!\n");
        uint16_t attrType;
        uint16_t attrLength;
        char sbcpPacket[BUFFER_SIZE];
        readAttribute(sbcpAttributes, sbcpPacket, offset, &attrType, &attrLength);
        printf("type = (%d), length = (%d)\n", attrType, attrLength);
        sbcpPacket[attrLength] = '\0';
        printf("Attribute 1 has payload : (%s)\n", sbcpPacket);

        switch ((enum headerTypes)type)
        {
            case Join:
                
        }

        offset = offset + HEADER_LENGTH + attrLength;
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
