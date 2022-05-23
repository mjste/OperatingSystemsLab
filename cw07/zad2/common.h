#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/time.h>

struct container
{
    int items;
    int read_index;
    int write_index;
    int space[5];
};

extern void sleep_miliseconds(long milis);
extern void sleep_range(int from, int to);
extern void set_sigint(void (*f)(int));