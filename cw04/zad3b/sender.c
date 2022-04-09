#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

enum commands
{
    CMD_ERR,
    CMD_KILL,
    CMD_SIGQUEUE,
    CMD_SIGRT
};

int get_n(char **argv);
int get_pid(char **argv);
int get_cmd(char **argv);
void handle(int signum, siginfo_t *infor, void *context);

int total_received = 0;

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "incorrect argc\n");
        exit(0);
    }
    pid_t pid = get_pid(argv);
    int n = get_n(argv);
    int cmd = get_cmd(argv);
    printf("pid: %d\n", getpid());

    struct sigaction siga;
    siga.sa_sigaction = handle;
    siga.sa_flags = SA_SIGINFO;
    sigfillset(&siga.sa_mask);

    sigaction(SIGUSR1, &siga, NULL);
    sigaction(SIGUSR2, &siga, NULL);
    sigaction(SIGRTMIN, &siga, NULL);
    sigaction(SIGRTMIN + 1, &siga, NULL);

    sigset_t ss;
    sigfillset(&ss);
    sigdelset(&ss, SIGUSR1);
    sigdelset(&ss, SIGUSR2);
    sigdelset(&ss, SIGRTMIN);
    sigdelset(&ss, SIGRTMIN + 1);
    sigdelset(&ss, SIGINT);

    switch (cmd)
    {
    case CMD_KILL:
        for (int i = 0; i < n; i++)
        {
            kill(pid, SIGUSR1);
            sigsuspend(&ss);
        }
        kill(pid, SIGUSR2);
        sigsuspend(&ss);
        break;
    case CMD_SIGQUEUE:
    {
        union sigval val;
        for (int i = 0; i < n; i++)
        {
            val.sival_int = i;
            sigqueue(pid, SIGUSR1, val);
            sigsuspend(&ss);
        }
        val.sival_int = n;
        sigqueue(pid, SIGUSR2, val);
        sigsuspend(&ss);
        break;
    }
    case CMD_SIGRT:
    {
        for (int i = 0; i < n; i++)
        {
            kill(pid, SIGRTMIN);
            sigsuspend(&ss);
        }
        kill(pid, SIGRTMIN + 1);
        sigsuspend(&ss);
        break;
    }
    }

    return 0;
}

int get_n(char **argv)
{
    if (isdigit(argv[2][0]))
    {
        return (atoi(argv[2]));
    }
    else
    {
        fprintf(stderr, "n is incorrect\n");
        exit(1);
    }
}

int get_pid(char **argv)
{
    if (isdigit(argv[1][0]))
    {
        return (atoi(argv[1]));
    }
    else
    {
        fprintf(stderr, "pid is incorrect\n");
        exit(1);
    }
}

void handle(int signum, siginfo_t *info, void *context)
{
    switch (signum)
    {
    case SIGUSR1:
        total_received++;
        printf("rc\n");
        break;
    case SIGUSR2:
        if (info->si_code == SI_QUEUE)
        {
            printf("catcher received %d\n", info->si_value.sival_int);
        }
        printf("received: %d\n", total_received);
        exit(0);
        break;
    default:
        if (signum == SIGRTMIN)
        {
            // printf("got\n");
            total_received++;
        }
        else if (signum == SIGRTMIN + 1)
        {
            if (info->si_code == SI_QUEUE)
            {
                printf("catcher received %d\n", info->si_value.sival_int);
            }
            printf("received: %d\n", total_received);
            exit(0);
        }
        break;
    }
}

int get_cmd(char **argv)
{
    char *arg = argv[3];
    if (strcmp(arg, "kill") == 0)
    {
        return CMD_KILL;
    }
    else if (strcmp(arg, "sigqueue") == 0)
    {
        return CMD_SIGQUEUE;
    }
    else if (strcmp(arg, "sigrt") == 0)
    {
        return CMD_SIGRT;
    }
    else
    {
        return CMD_ERR;
    }
}