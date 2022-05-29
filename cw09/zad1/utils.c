#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "utils.h"

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