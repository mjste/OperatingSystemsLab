#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

enum command
{
    CMD_ERR,
    CMD_IGNORE,
    CMD_HANDLE,
    CMD_MASK,
    CMD_PENDING,
    CMD_FORK,
    CMD_EXEC
};

int mapString(char *s);
void sighandler(int signum);

int main(int argc, char **argv)
{
    int cmd_mode = 0;
    switch (argc)
    {
    case 2:
    {
        cmd_mode = CMD_FORK;

        break;
    }
    case 3:
    {
        cmd_mode = CMD_EXEC;
        break;
    }
    default:
    {
        fprintf(stderr, "bad argument number\n");
        exit(1);
    }
    }

    printf("pid: %d\n", getpid());
    int cmd = mapString(argv[1]);
    switch (cmd)
    {
    case CMD_HANDLE:
    {
        signal(SIGUSR1, sighandler);
        raise(SIGUSR1);

        pid_t childpid;
        if ((childpid = fork()) == 0)
        {
            if (cmd_mode == CMD_EXEC)
            {
                char *args[2] = {"./EXEC", NULL};
                execvp(args[0], args);
                printf("this won't show\n");
            }
            raise(SIGUSR1);
            exit(0);
        }
        if (cmd_mode == CMD_EXEC)
        {
            sleep(1);
            kill(childpid, SIGUSR1);
        }
        wait(NULL);
        break;
    }
    case CMD_IGNORE:
    {
        signal(SIGUSR1, SIG_IGN);
        raise(SIGUSR1);

        pid_t childpid;
        if ((childpid = fork()) == 0)
        {
            if (cmd_mode == CMD_EXEC)
            {
                char *args[2] = {"./EXEC", NULL};
                execvp(args[0], args);
                printf("this won't show\n");
            }
            raise(SIGUSR1);
            exit(0);
        }
        if (cmd_mode == CMD_EXEC)
        {
            sleep(1);
            kill(childpid, SIGUSR1);
        }
        wait(NULL);
        break;
    }
    case CMD_MASK:
    {
        sigset_t new_mask;
        sigset_t old_mask;

        sigemptyset(&new_mask);
        sigaddset(&new_mask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &new_mask, &old_mask);

        signal(SIGUSR1, sighandler);
        printf("00\t");
        raise(SIGUSR1);
        printf("01\n");

        pid_t childpid;
        if ((childpid = fork()) == 0)
        {
            if (cmd_mode == CMD_EXEC)
            {
                char *args[2] = {"./EXEC", NULL};
                execvp(args[0], args);
                printf("this won't show\n");
            }
            printf("10\t");
            raise(SIGUSR1);
            printf("11\n");
            exit(0);
        }
        if (cmd_mode == CMD_EXEC)
        {
            sleep(1);
            kill(childpid, SIGUSR1);
        }
        wait(NULL);
        printf("Done\n");
        break;
    }
    case CMD_PENDING:
    {
        sigset_t new_mask;
        sigset_t old_mask;

        sigemptyset(&new_mask);
        sigaddset(&new_mask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &new_mask, &old_mask);
        signal(SIGUSR1, sighandler);
        raise(SIGUSR1);
        sigset_t wait_mask;
        sigpending(&wait_mask);
        printf("Parent. Pending?: %d\n", sigismember(&wait_mask, SIGUSR1));

        pid_t childpid;
        if ((childpid = fork()) == 0)
        {
            if (cmd_mode == CMD_EXEC)
            {
                char *args[2] = {"./EXEC", NULL};
                execvp(args[0], args);
                printf("this won't show\n");
            }
            sigpending(&wait_mask);
            printf("Child. Pending?: %d\n", sigismember(&wait_mask, SIGUSR1));
            exit(0);
        }
        wait(NULL);
        printf("Done\n");
        break;
    }
    }

    return 0;
}

int mapString(char *s)
{
    if (strcmp(s, "ignore") == 0)
    {
        return CMD_IGNORE;
    }
    else if (strcmp(s, "handle") == 0)
    {
        return CMD_HANDLE;
    }
    else if (strcmp(s, "mask") == 0)
    {
        return CMD_MASK;
    }
    else if (strcmp(s, "pending") == 0)
    {
        return CMD_PENDING;
    }
    else
    {
        return CMD_ERR;
    }
}

void sighandler(int sig)
{
    printf("receive signal: %d, pid: %d\n", sig, getpid());
}