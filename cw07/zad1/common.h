#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/times.h>
#include <sys/time.h>

enum place_sem
{
    OVEN,
    TABLE
};

struct container
{
    int items;
    int read_index;
    int write_index;
    int space[5];
};

extern void wait_for_sem(int sem_set_id, int sem_no);
extern void free_sem(int sem_set_id, int sem_no);
