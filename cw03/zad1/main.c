#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "main: please use 1 argument\n");
        exit(1);
    }
    if (!isdigit(argv[1][0])) {
        fprintf(stderr, "main: please enter a number\n");
        exit(2);
    }

    int n = atoi(argv[1]);
    pid_t child_pid;

    printf("Main process\n");
    for (int i = 0; i < n; i++) {
        child_pid = fork();
        if (child_pid == 0) {
            printf("Printf from  child process %d\n", (int)getpid());
            return 0;
        }
    }
    for(int i = 0; i< n; i++)
        wait(NULL);

    return 0;
}