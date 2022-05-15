#define MESSAGE_SIZE 2000
#define SERVER_KEY_NUMBER 1

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

struct packet_message
{
    unsigned char sender_id;
    unsigned char receiver_id;
    int queue_id;
    char message[MESSAGE_SIZE];
};

struct packet
{
    long type;
    struct packet_message packet_message;
};
