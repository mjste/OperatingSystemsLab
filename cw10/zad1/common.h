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

#define PORT_NUMBER 24000
#define SOCKET_PATH "/tmp/us_xfr"
#define NAME_LENGTH 30

enum play_sign
{
    CROSS,
    CIRCLE
};