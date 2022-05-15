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
    char buffer[200];

    char qname[20] = "/kolejka";
    mqd_t queue;
    if ((queue = mq_open(qname, O_RDONLY)) == -1)
    {
        perror("otwieranie kolejki");
        exit(errno);
    }

    sleep(4);
    for (int i = 0; i < 20; i++)
    {
        if (mq_receive(queue, buffer, sizeof(buffer), 0) > 0)
        {
            puts(buffer);
        }
    }

    while (1)
    {
        sleep(1);
    }

    mq_close(queue);

    return 0;
}