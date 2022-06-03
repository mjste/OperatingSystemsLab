#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

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
    ser.sin_port = htons(24001);
    ser.sin_addr.s_addr = inet_addr("127.0.0.1"); // htonl(INADDR_ANY)

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
        if (buf[0] == 'Q')
        {
            sprintf(buf, "ZGODA, SERWER KONCZY PRACE\n");
            end = 0;
        }
        else if (buf[0] == 'N')
        {
            sprintf(buf, "Jestes klienter nr %d", nr++);
        }
        else
        {
            sprintf(buf, "Nie rozumiem pytania");
        }
        write(gniazdo2, buf, strlen(buf));
        close(gniazdo2);
    }

    return 0;
}

// można testować tak:
// $ telnet 127.0.0.1 <numer portu>