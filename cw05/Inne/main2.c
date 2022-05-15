#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>

int main()
{
    int fd[2];
    pipe(fd);
    pid_t pid = fork();
    if (pid == 0)
    { // dziecko
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        execlp("grep", "grep", "Ala", NULL);
    }
    else
    { // rodzic
        close(fd[0]);
        char text[] = "0 foo bar\n1 Ala ma kota\n2 fagag";
        write(fd[1], text, strlen(text));
        close(fd[1]);
    }

    //     FILE* grep_input = popen("grep Ala", "w");
    // // fputs(..., grep_input) - przesłanie danych do grep-a
    // pclose(grep_input);
    return 0;
}