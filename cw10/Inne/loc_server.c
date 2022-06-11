#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stdlib.h>

#define BACKLOG 5
#define SV_SOCK_PATH "/tmp/us_xfr"
#define BUF_SIZE 256

int main(int argc, char *argv[])
{
    struct sockaddr_un addr;

    // Create a new server socket with domain: AF_UNIX, type: SOCK_STREAM, protocol: 0
    int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    printf("Server socket fd = %d\n", sfd);

    // Make sure socket's file descriptor is legit.
    if (sfd == -1)
    {
        perror("socket");
        exit(0);
    }

    // Make sure the address we're planning to use isn't too long.
    if (strlen(SV_SOCK_PATH) > sizeof(addr.sun_path) - 1)
    {
        printf("name too long\n");
        exit(0);
    }

    // Delete any file that already exists at the address. Make sure the deletion
    // succeeds. If the error is just that the file/directory doesn't exist, it's fine.
    if (remove(SV_SOCK_PATH) == -1 && errno != ENOENT)
    {
        perror("remove");
        exit(0);
    }

    // Zero out the address, and set family and path.
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

    // Bind the socket to the address. Note that we're binding the server socket
    // to a well-known address so that clients know where to connect.
    if (bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1)
    {
        perror("bind");
        exit(0);
    }

    // The listen call marks the socket as *passive*. The socket will subsequently
    // be used to accept connections from *active* sockets.
    // listen cannot be called on a connected socket (a socket on which a connect()
    // has been succesfully performed or a socket returned by a call to accept()).
    if (listen(sfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(0);
    }

    ssize_t numRead;
    char buf[BUF_SIZE];
    for (;;)
    { /* Handle client connections iteratively */

        // Accept a connection. The connection is returned on a NEW
        // socket, 'cfd'; the listening socket ('sfd') remains open
        // and can be used to accept further connections. */
        printf("Waiting to accept a connection...\n");
        // NOTE: blocks until a connection request arrives.
        int cfd = accept(sfd, NULL, NULL);
        printf("Accepted socket fd = %d\n", cfd);

        //
        // Transfer data from connected socket to stdout until EOF */
        //

        // Read at most BUF_SIZE bytes from the socket into buf.
        while ((numRead = read(cfd, buf, BUF_SIZE)) > 0)
        {
            // Then, write those bytes from buf into STDOUT.
            if (write(STDOUT_FILENO, buf, numRead) != numRead)
            {
                printf("fatal/partial write\n");
                exit(0);
            }
        }

        if (numRead == -1)
        {
            perror("read");
            exit(0);
        }

        if (close(cfd) == -1)
        {
            perror("close");
            exit(0);
        }
    }
}