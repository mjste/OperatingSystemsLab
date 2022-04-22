#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>

int main(int argc, char **argv)
{
    /*
    1 - fifo path
    2 - line number
    3 - filepath
    4 - N - number of read characters
    */
    if (argc != 5)
    {
        fprintf(stderr, "Wrong argument number\n");
        exit(1);
    }
    if (!isdigit(argv[2][0]))
    {
        fprintf(stderr, "Line must be a number\n");
        exit(1);
    }
    if (!isdigit(argv[4][0]))
    {
        fprintf(stderr, "Read bytes must be a number\n");
        exit(1);
    }
    int fp = open(argv[3], O_RDONLY);
    if (fp == -1)
    {
        perror(argv[3]);
        exit(1);
    }
    int fifop = open(argv[1], O_WRONLY);
    if (fifop == -1)
    {
        perror(argv[1]);
        exit(1);
    }
    int line_number = atoi(argv[2]);
    int block_size = atoi(argv[4]);
    if (line_number > 9999 || line_number < 0)
    {
        fprintf(stderr, "Line number must be 0 <= n <= 9999");
        exit(1);
    }
    if (block_size > PIPE_BUF - 4 || block_size <= 0)
    {
        fprintf(stderr, "Block size is too big");
        exit(1);
    }
    // Error handling done

    char buffer[PIPE_BUF];
    struct timespec timespec0;
    struct timespec timespec1;
    srand(time(NULL));

    int bytes = 0;
    while (1)
    {
        // sleep random time (0-1s)
        timespec0.tv_sec = 0;
        timespec0.tv_nsec = (rand() % 1000000000);
        nanosleep(&timespec0, &timespec1);

        // clean buffer
        for (int i = 0; i < PIPE_BUF; i++)
            buffer[i] = 0;

        sprintf(buffer, "%-4d", line_number);
        bytes = read(fp, buffer + 4, block_size);
        if (bytes == 0)
            break;
        write(fifop, buffer, bytes + 4);
    }

    close(fp);
    close(fifop);
    return 0;
}