#include "common.h"

void fatal(char *message)
{
    perror(message);
    exit(-1);
}