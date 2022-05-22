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