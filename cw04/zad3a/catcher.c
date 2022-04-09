#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void handle(int signum, siginfo_t *info, void *context);
void set_mask();

int total_count = 0;

int main(int argc, char **argv)
{
    printf("pid: %d\n", getpid());
    struct sigaction siga;
    siga.sa_sigaction = handle;
    siga.sa_flags = SA_SIGINFO;
    sigfillset(&siga.sa_mask);

    sigaction(SIGUSR1, &siga, NULL);
    sigaction(SIGUSR2, &siga, NULL);
    sigaction(SIGRTMIN, &siga, NULL);
    sigaction(SIGRTMIN + 1, &siga, NULL);

    set_mask();

    sigset_t ss;
    sigfillset(&ss);
    sigdelset(&ss, SIGUSR1);
    sigdelset(&ss, SIGUSR2);
    sigdelset(&ss, SIGRTMIN);
    sigdelset(&ss, SIGRTMIN + 1);
    sigdelset(&ss, SIGINT);
    while (1)
        sigsuspend(&ss);

    return 0;
}

void handle(int signum, siginfo_t *info, void *context)
{
    pid_t pid = info->si_pid;
    switch (signum)
    {
    case SIGUSR1:
        total_count++;
        printf("Got signal usr1\n");
        break;
    case SIGUSR2:
        printf("Got signal usr2\n");
        printf("Got %d usr1\n", total_count);

        for (int i = 0; i < total_count; i++)
            kill(pid, SIGUSR1);

        // sleep(1);
        if (info->si_code == SI_QUEUE)
        {
            union sigval sigval;
            sigval.sival_int = total_count;
            sigqueue(pid, SIGUSR2, sigval);
        }
        else
        {
            kill(pid, SIGUSR2);
        }
        total_count = 0;
        exit(0);
        break;
    default:
        if (signum == SIGRTMIN)
        {
            total_count++;
        }
        else
        {
            // printf("Got signal rt2\n");
            printf("Got %d rt1\n", total_count);

            for (int i = 0; i < total_count; i++)
            {
                kill(pid, SIGRTMIN);
            }
            union sigval sigval;
            sigval.sival_int = total_count;
            sigqueue(pid, SIGRTMIN + 1, sigval);
            total_count = 0;
            exit(0);
        }
    }
}

void set_mask()
{
    sigset_t nmask;
    sigfillset(&nmask);
    sigprocmask(SIG_BLOCK, &nmask, NULL);
}