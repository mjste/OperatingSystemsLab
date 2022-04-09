#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
    int fd[2];
    pipe(fd);
    pid_t pid = fork();
    if (pid == 0)
    { // dziecko
        close(fd[1]);
        char buffer[100];
        read(fd[0], buffer, 5);
        printf("%s", buffer);
    }
    else
    { // rodzic
        close(fd[0]);
        write(fd[1], "abc\n", 5);
    }
    return 0;
}
