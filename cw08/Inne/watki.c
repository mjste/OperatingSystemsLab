#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

pthread_t watek01;
pthread_mutex_t mutex01 = PTHREAD_MUTEX_INITIALIZER;

void *fun_watka(void *nic)
{
    struct tm *t1;
    char buf[15];
    time_t t;

    while (1)
    {
        t = time(NULL);
        t1 = localtime(&t);
        strftime(buf, 10, "%H:%M:%S", t1);
        buf[8] = 0;
        pthread_mutex_lock(&mutex01);
        printf("\33[1;73H");
        printf("%s", buf);
        printf("\33[25;1h");
        fflush(stdout);
        pthread_mutex_unlock(&mutex01);
        sleep(1);
    }
}

int main()
{
    int i;
    pthread_create(&watek01, NULL, &fun_watka, NULL);
    for (i = 1; i < 100; i++)
    {
        pthread_mutex_lock(&mutex01);
        printf("%d", i);
        fflush(stdout);
        pthread_mutex_unlock(&mutex01);
        sleep(3);
    }

    return 0;
}