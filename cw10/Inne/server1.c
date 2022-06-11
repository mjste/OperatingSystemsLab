#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define PORT_NUMBER 24001

char reply[] = "HTTP/1.1 200 OK\r\nContent-Length: 136\r\nContent-Type: text/html\r\nConnection: Closed\r\n\r\n<!DOCTYPE html><html><head><meta charset=\"UTF-8\"></head><body><h1>Nudny serwer, jedyne co robi to odpisuje t\xc4\x99 wiadomość</h1></body></html>";

int main()
{
    int status, gniazdo, dlogosc, nr = 0, end = 1, gniazdo2;
    struct sockaddr_in ser, cli;
    char buf[200];

    gniazdo = socket(AF_INET, SOCK_STREAM, 0);
    if (gniazdo == -1)
    {
        printf("blad socket\n");
        return 0;
    }

    memset(&ser, 0, sizeof(ser));
    ser.sin_family = AF_INET;
    ser.sin_port = htons(PORT_NUMBER);
    // ser.sin_addr.s_addr = inet_addr("127.0.0.1"); // htonl(INADDR_ANY)
    ser.sin_addr.s_addr = htonl(INADDR_ANY);

    status = bind(gniazdo, (struct sockaddr *)&ser, sizeof(ser));
    if (status == -1)
    {
        printf("blad bind\n");
        return 0;
    }

    status = listen(gniazdo, 10);
    if (status == -1)
    {
        printf("blad listen\n");
        return 0;
    }

    while (end)
    {
        dlogosc = sizeof(cli);
        gniazdo2 = accept(gniazdo, (struct sockaddr *)&cli, (socklen_t *)&dlogosc);
        if (gniazdo2 == -1)
        {
            printf("blad accept\n");
            return 0;
        }
        read(gniazdo2, buf, sizeof(buf));
        // sprintf(buf, "<!DOCTYPE html><html><head></head><body>nudny serwer</body></html>");
        // write(gniazdo2, buf, strlen(buf));
        write(gniazdo2, reply, strlen(reply));
        close(gniazdo2);
    }

    return 0;
}

// można testować tak:
// $ telnet 127.0.0.1 <numer portu>