#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define SV_SOCK_PATH "/tmp/us_xfr"
#define BUFF_SIZE 256

int main()
{
    struct sockaddr_un addr;
    ssize_t numRead;
    char buf[BUFF_SIZE];

    // Create a new client socket with domain: AF_UNIX, type: SOCK_STREAM, protocol: 0
    int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    printf("Client socket fd = %d\n", sfd);

    // Make sure socket's file descriptor is legit.
    if (sfd == -1)
    {
        perror("socket");
        exit(0);
    }

    //
    // Construct server address, and make the connection.
    //
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

    // Connects the active socket referred to be sfd to the listening socket
    // whose address is specified by addr.
    if (connect(sfd, (struct sockaddr *)&addr,
                sizeof(struct sockaddr_un)) == -1)
    {
        perror("connect");
        exit(0);
    }

    //
    // Copy stdin to socket.
    //

    // Read at most BUF_SIZE bytes from STDIN into buf.
    while ((numRead = read(STDIN_FILENO, buf, BUFF_SIZE)) > 0)
    {
        // Then, write those bytes from buf into the socket.
        if (write(sfd, buf, numRead) != numRead)
        {
            printf("partial/failed write\n");
        }
    }

    if (numRead == -1)
    {
        perror("read");
        exit(0);
    }

    // Closes our socket; server sees EOF.
    exit(EXIT_SUCCESS);
}