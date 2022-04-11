#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, const char *argv[])
{
    int in = open("cin.log", O_RDONLY | O_CREAT, 0600);
    if (in == -1)
    {
        perror("opening cin.log");
        return 255;
    }

    int out = open("cout.log", O_RDWR | O_CREAT | O_APPEND, 0600);
    if (-1 == out)
    {
        perror("opening cout.log");
        return 255;
    }

    int err = open("cerr.log", O_RDWR | O_CREAT | O_APPEND, 0600);
    if (-1 == err)
    {
        perror("opening cerr.log");
        return 255;
    }

    int save_in = dup(fileno(stdin));
    int save_out = dup(fileno(stdout));
    int save_err = dup(fileno(stderr));

    if (-1 == dup2(in, fileno(stdin)))
    {
        perror("cannot redirect stdin");
        return 255;
    }
    if (-1 == dup2(out, fileno(stdout)))
    {
        perror("cannot redirect stdout");
        return 255;
    }
    if (-1 == dup2(err, fileno(stderr)))
    {
        perror("cannot redirect stderr");
        return 255;
    }

    int num[3];
    scanf("%d", num);
    scanf("%d", num + 1);
    scanf("%d", num + 2);

    printf("%d %d %d\n", num[0], num[1], num[2]);
    puts("doing an ls or something now");

    fflush(stdout);
    close(out);
    fflush(stderr);
    close(err);

    dup2(save_out, fileno(stdout));
    dup2(save_err, fileno(stderr));
    dup2(save_in, fileno(stdin));

    close(save_out);
    close(save_err);
    close(save_in);

    puts("back to normal output");

    return 0;
}