#include "common.h"

void wait_for_sem(int sem_set_id, int sem_no)
{
    struct sembuf sem_buf;
    sem_buf.sem_num = sem_no;
    sem_buf.sem_op = -1;
    sem_buf.sem_flg = 0;
    semop(sem_set_id, &sem_buf, 1); // wait for access
}

void free_sem(int sem_set_id, int sem_no)
{
    struct sembuf sem_buf;
    sem_buf.sem_num = sem_no;
    sem_buf.sem_op = 1;
    sem_buf.sem_flg = 0;
    semop(sem_set_id, &sem_buf, 1);
}

void sleep_miliseconds(long milis)
{
    struct timespec ts;
    ts.tv_sec = milis / 1000;
    ts.tv_nsec = (milis % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

void sleep_range(int from, int to)
{
    long range = rand() % ((to - from) * 1000);
    sleep(from);
    sleep_miliseconds(range);
}