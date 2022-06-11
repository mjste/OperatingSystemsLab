#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define PORT 80
#define BUF_SIZE 1000

int main(int argc, char **argv)
{
    int socket_id, status;
    struct sockaddr_in saddr;
    char buf[BUF_SIZE];
    socket_id = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_id == -1)
    {
        printf("blad socket\n");
        exit(0);
    }

    // podaj ip
    char ip_string[20];
    printf("Podaj ip: ");
    scanf("%s", ip_string);

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(PORT);
    saddr.sin_addr.s_addr = inet_addr(ip_string);

    printf("Dzięki\n");
    status = connect(socket_id, (struct sockaddr *)&saddr, sizeof(saddr));
    if (status == -1)
    {
        printf("Blad connect\n");
    }
    sprintf(buf, "GET / HTTP/1.0\r\n\r\n");
    write(socket_id, buf, strlen(buf));
    memset(buf, 0, BUF_SIZE);

    while (read(socket_id, buf, BUF_SIZE - 1) > 0)
    {
        printf("%s\n", buf);
    }

    if (close(socket_id) == -1)
    {
        perror("closing");
        exit(0);
    }

    return 0;
}