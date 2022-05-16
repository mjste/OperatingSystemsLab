#ifndef _COMMON_HEADER
#include <sys/types.h>
#include <sys/ipc.h>
#define MESSAGE_SIZE 200
#define PACKET_SIZE (MESSAGE_SIZE + 3)
#define SERVER_KEY_NUMBER 0

enum message_fields
{
    MF_TYPE,
    MF_FROM,
    MF_TO,
    MF_MESSAGE
};

enum requests
{
    ANY_REQUEST,
    CS_LIST,
    CS_TO_ALL,
    CS_TO_ONE,
    CS_STOP,
    CS_INIT,
    SC_LIST,
    SC_SEND,
    SC_STOP,
    SC_INIT,
    C_READ,
    UNKNOWN_REQUEST
};

#endif