#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
    mqd_t mq;
    int i;

    mq = mq_open("nazwa_kolejki", O_RDONLY);
    if (mq == -1)
    {
        perror("opening queue");
    }
    while (1)
    {
        if (mq_receive(mq, (char *)&i, sizeof(int), NULL) < 0)
            break;
        printf("otrzymano : %d", i);
    }
    mq_close(mq);
    // mq_unlink(QUEUE_NAME);
    return 0;
}