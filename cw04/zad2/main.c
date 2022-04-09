#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

void handler1(int signum, siginfo_t *info, void *context)
{
    // siginfo pozwala na uzyskanie dodatkowych informacji
    printf("Signal sent from process %d user: %d\n", info->si_pid, info->si_uid);
    // siginfo_t {
    //                int      si_signo;     /* Signal number */
    //                int      si_errno;     /* An errno value */
    //                int      si_code;      /* Signal code */
    //                int      si_trapno;    /* Trap number that caused
    //                                          hardware-generated signal
    //                                          (unused on most architectures) */
    //                pid_t    si_pid;       /* Sending process ID */
    //                uid_t    si_uid;       /* Real user ID of sending process */
    //                int      si_status;    /* Exit value or signal */
    //                clock_t  si_utime;     /* User time consumed */
    //                clock_t  si_stime;     /* System time consumed */
    //                sigval_t si_value;     /* Signal value */
    //                int      si_int;       /* POSIX.1b signal */
    //                void    *si_ptr;       /* POSIX.1b signal */
    //                int      si_overrun;   /* Timer overrun count;
    //                                          POSIX.1b timers */
    //                int      si_timerid;   /* Timer ID; POSIX.1b timers */
    //                void    *si_addr;      /* Memory location which caused fault */
    //                long     si_band;      /* Band event (was int in
    //                                          glibc 2.3.2 and earlier) */
    //                int      si_fd;        /* File descriptor */
    //                short    si_addr_lsb;  /* Least significant bit of address
    //                                          (since Linux 2.6.32) */
    //                void    *si_lower;     /* Lower bound when address violation
    //                                          occurred (since Linux 3.19) */
    //                void    *si_upper;     /* Upper bound when address violation
    //                                          occurred (since Linux 3.19) */
    //                int      si_pkey;      /* Protection key on PTE that caused
    //                                          fault (since Linux 4.6) */
    //                void    *si_call_addr; /* Address of system call instruction
    //                                          (since Linux 3.5) */
    //                int      si_syscall;   /* Number of attempted system call
    //                                          (since Linux 3.5) */
    //                unsigned int si_arch;  /* Architecture of attempted system call
    //                                          (since Linux 3.5) */
    //            }
}

void test1()
{
    struct sigaction siga;
    siga.sa_flags = SA_SIGINFO;
    sigemptyset(&siga.sa_mask);
    siga.sa_sigaction = handler1;
    sigaction(SIGUSR1, &siga, NULL);

    raise(SIGUSR1);
    printf("test1 done\n");
}

void handler2(int signum)
{
    printf("child terminated/stopped\n");
}

void test2()
{
    // resethand resets signal handling after first use
    struct sigaction siga;
    sigemptyset(&siga.sa_mask);
    siga.sa_handler = handler2;
    siga.sa_flags = SA_RESETHAND;
    sigaction(SIGCHLD, &siga, NULL);

    if (fork() == 0)
    {
        printf("inside\n");
        exit(0);
    }
    sleep(1);
    if (fork() == 0)
    {
        printf("inside\n");
        exit(0);
    }
    sleep(1);
    wait(NULL);
    printf("test2 done\n");
}

void test3()
{
    // resethand resets signal handling after first use
    struct sigaction siga;
    sigemptyset(&siga.sa_mask);
    siga.sa_handler = handler2;

    // siga.sa_flags = SA_NOCLDSTOP;
    sigaction(SIGCHLD, &siga, NULL);

    pid_t childpid;
    if ((childpid = fork()) == 0)
    {
        printf("1\n");
        printf("2\n");
        exit(0);
    }
    kill(childpid, SIGSTOP);
    sleep(1);
    kill(childpid, SIGCONT);

    printf("test3 done\n");
}

int main(int argc, char **argv)
{
    test1();
    test2();
    test3();

    return 0;
}