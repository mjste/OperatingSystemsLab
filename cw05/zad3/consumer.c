#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>

#define LINES 128
#define LINELEN 1024

struct line
{
    int index;
    char chars[LINELEN];
};

FILE *fp;
// int fp;
int fifop;
struct line lines[LINES];

void handle(int signum)
{
    for (int i = 0; i < LINES; i++)
    {
        fprintf(fp, "%s\n", lines[i].chars);
    }
    fclose(fp);
    close(fifop);
    exit(0);
}

int main(int argc, char **argv)
{
    /*
    1 - fifo path
    2 - outfile path
    3 - block_size
    */
    if (argc != 4)
    {
        fprintf(stderr, "Wrong arguments count\n");
        exit(1);
    }
    fp = fopen(argv[2], "w");
    // fp = open(argv[2], O_WRONLY);
    if (fp == NULL)
    {
        perror(argv[2]);
        exit(1);
    }
    if (!isdigit(argv[3][0]))
    {
        fprintf(stderr, "Not a number\n");
        exit(1);
    }
    fifop = open(argv[1], O_RDONLY);
    if (fifop == -1)
    {
        perror(argv[1]);
        exit(1);
    }
    int block_size = atoi(argv[3]);
    if (block_size <= 0 || block_size > PIPE_BUF - 4)
    {
        fprintf(stderr, "Wrong block size\n");
        exit(1);
    }

    // init lines
    for (int i = 0; i < LINES; i++)
    {
        lines[i].index = 0;
        for (int j = 0; j < LINELEN; j++)
            lines[i].chars[j] = 0;
    }

    // add SIGINT as signal to safe exit
    struct sigaction siga;
    siga.sa_flags = 0;
    sigemptyset(&siga.sa_mask);
    siga.sa_handler = handle;
    sigaction(SIGINT, &siga, NULL);

    char buffer[PIPE_BUF];
    while (1)
    {
        for (int i = 0; i < PIPE_BUF; i++)
            buffer[i] = 0;
        // char numbuf[5];
        // for (int i = 0; i < 5; i++)
        //     numbuf[i] = 0;
        // read(fifop, numbuf, 4);
        // int lineno = atoi(numbuf);

        // bytes read 4, then bytes read other !!!! TODO
        int bytes = read(fifop, buffer, block_size + 4);
        char numbuf[5];
        for (int i = 0; i < 4; i++)
            numbuf[i] = buffer[i];
        numbuf[4] = 0;
        int lineno = atoi(numbuf);

        int index = lines[lineno].index;
        for (int i = 0; i < bytes - 4; i++)
        {
            lines[lineno].chars[index++] = buffer[i + 4];
        }
        lines[lineno].index = index;

        if (bytes != 0)
        {
            for (int i = 0; i < block_size + 4; i++)
            {
                printf("%d, ", (int)buffer[i]);
            }

            // fwrite(buffer, 1, block_size + 4, stdout);
            printf("\t\t%d\n", bytes);
        }
    }

    return 0;
}