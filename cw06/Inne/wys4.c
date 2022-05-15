#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    char buffer[200] = "jakas wiadomosc";

    char qname[20] = "/kolejka";
    struct mq_attr mqattr;
    mqattr.mq_flags = 0;
    mqattr.mq_msgsize = 200;
    mqattr.mq_curmsgs = 0;
    mqattr.mq_maxmsg = 10; // więcej nie można bez zmiany plików systemowych

    mqd_t queue;
    if ((queue = mq_open(qname, O_WRONLY | O_CREAT, 0644, &mqattr)) == -1)
    {
        perror("tworzenie kolejki");
        exit(errno);
    }

    for (int i = 0; i < 20; i++)
    {
        if (mq_send(queue, buffer, sizeof(buffer), 0) == -1)
        {
            perror("sending");
            exit(errno);
        }
    }

    if (mq_close(queue) == -1)
    {
        perror("zamykanie");
        exit(errno);
    }

    // while (1)
    // {
    //     sleep(1);
    // }

    sleep(2);

    if (mq_unlink(qname) == -1)
    {
        perror("odlinkowanie");
        exit(errno);
    }

    return 0;
}