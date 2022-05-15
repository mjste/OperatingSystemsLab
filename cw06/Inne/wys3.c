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
    struct mq_attr atr;
    int i;
    atr.mq_flags = 0;
    atr.mq_maxmsg = 100;
    atr.mq_msgsize = sizeof(int);
    atr.mq_curmsgs = 0;
    mqd_t kolejka_kom;

    if ((kolejka_kom = mq_open("nazwa_kolejki", O_CREAT | O_WRONLY, 0644, &atr)) == -1)
    {
        perror("otwieranie kolejki");
        exit(-1);
    }

    if ((kolejka_kom = mq_open("nazwa_kolejki", O_WRONLY | O_CREAT, &atr)) == -1)
    {
        perror("otwieranie kolejki ;(");
        exit(-1);
    }

    for (i = 0; i < 20; i++)
    {
        if (mq_send(kolejka_kom, (const char *)&i, sizeof(int), 0) == -1)
        {
            perror("blad");
            exit(-1);
        }
    }
    mq_close(kolejka_kom);
    return 0;
}