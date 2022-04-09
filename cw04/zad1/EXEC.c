#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main()
{
    printf("I am EXEC\n");
    sleep(2);
    printf("message after signal (-> ignored/blocked)\n");
    sigset_t mask;
    sigpending(&mask);
    printf("Pending?: %d\n", sigismember(&mask, SIGUSR1));

    return 0;
}