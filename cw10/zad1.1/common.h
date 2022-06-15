#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>     // socketaddr_un
#include <netinet/in.h> // socketaddr_in
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <limits.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#define NAME_LENGTH 30
#define BUFFER_SIZE 40
#define READ_SIZE (BUFFER_SIZE - 1)

enum message_s
{
    CROSS = 10,
    CIRCLE,
    WAIT,
    PLAY,
    WRONG_MESSAGE,
    QUIT,
    FIRST,
    SECOND,
    WIN,
    DEFEAT,
    PING,
    DRAW
};

void fatal(char *message);