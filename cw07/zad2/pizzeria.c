#include "common.h"

void sigint_handler(int signum);
void process_arguments(char **argv, int *np, int *mp);

int running = 1;

int main(int argc, char **argv)
{
    set_sigint(sigint_handler);
    if (argc != 3)
    {

        printf("Usage: pizzeria <n> <m>\n");
        exit(-1);
    }
    int n, m;
    process_arguments(argv, &n, &m);

    // open semaphores
    sem_t *oven_access_sem = sem_open("/oven_access", O_CREAT | O_EXCL, 0600, 1);
    if (oven_access_sem == SEM_FAILED)
    {
        perror("oven_access_sem");
        exit(-1);
    }
    sem_t *oven_count_sem = sem_open("/oven_count", O_CREAT | O_EXCL, 0600, 5);
    if (oven_count_sem == SEM_FAILED)
    {
        perror("oven_count_sem");
        exit(-1);
    }
    sem_t *table_access_sem = sem_open("/table_access", O_CREAT | O_EXCL, 0600, 1);
    if (table_access_sem == SEM_FAILED)
    {
        perror("table_access_sem");
        exit(-1);
    }
    sem_t *table_count_sem = sem_open("/table_count", O_CREAT | O_EXCL, 0600, 5);
    if (table_count_sem == SEM_FAILED)
    {
        perror("table_count_sem");
        exit(-1);
    }

    // open shared memory
    int oven_mem_id = shm_open("/oven_mem", O_RDWR | O_CREAT | O_EXCL, 0600);
    if (oven_mem_id == -1)
    {
        perror("oven_mem");
        exit(-1);
    }
    int table_mem_id = shm_open("/table_mem", O_RDWR | O_CREAT | O_EXCL, 0600);
    if (table_mem_id == -1)
    {
        perror("table_mem_id");
        exit(-1);
    }
    // truncate
    int oven_trunc = ftruncate(oven_mem_id, sizeof(struct container));
    if (oven_trunc == -1)
    {
        perror("oven_trunc");
        exit(-1);
    }
    int table_trunc = ftruncate(table_mem_id, sizeof(struct container));
    if (table_trunc == -1)
    {
        perror("table_trunc");
        exit(-1);
    }
    // run bakers
    for (int i = 0; i < n; i++)
    {
        pid_t childpid = fork();
        if (childpid == 0)
        {
            execl("./baker", "./baker", NULL);
        }
    }
    // run deliverers
    for (int i = 0; i < m; i++)
    {
        pid_t childpid = fork();
        if (childpid == 0)
        {
            execl("./deliverer", "./deliverer", NULL);
        }
    }

    // wait
    while (running)
        sleep(1);

    // close semaphores
    if (sem_close(oven_access_sem) == -1)
    {
        perror("closing oven_access_sem");
        exit(-1);
    }
    if (sem_close(oven_count_sem) == -1)
    {
        perror("closing oven_count_sem");
        exit(-1);
    }
    if (sem_close(table_access_sem) == -1)
    {
        perror("Closing table_access_sem");
        exit(-1);
    }
    if (sem_close(table_count_sem) == -1)
    {
        perror("Closing table_count_sem");
        exit(-1);
    }

    // unlink semaphores
    if (sem_unlink("/oven_access") == -1)
    {
        perror("unlinking oven_access_sem");
        exit(-1);
    }
    if (sem_unlink("/oven_count") == -1)
    {
        perror("unlinking oven_count_sem");
        exit(-1);
    }
    if (sem_unlink("/table_access") == -1)
    {
        perror("unlinking table_access_sem");
        exit(-1);
    }
    if (sem_unlink("/table_count") == -1)
    {
        perror("unlinking table_count_sem");
        exit(-1);
    }

    // close_shared_memory
    if (close(oven_mem_id) == -1)
    {
        perror("closing_oven_handle");
        exit(-1);
    }
    if (close(table_mem_id) == -1)
    {
        perror("closing_table_handle");
        exit(-1);
    }

    // unlink shared memory
    if (shm_unlink("/oven_mem") == -1)
    {
        perror("oven_unlink");
        exit(-1);
    }
    if (shm_unlink("/table_mem") == -1)
    {
        perror("table_unlink");
        exit(-1);
    }

    return 0;
}

void sigint_handler(int signum)
{
    running = 0;
}

void process_arguments(char **argv, int *np, int *mp)
{
    if (!isdigit(argv[1][0]))
    {
        printf("Pierwszy argument musi być liczbą\n");
        exit(-1);
    }

    if (!isdigit(argv[2][0]))
    {
        printf("Drugi argument musi być liczbą\n");
        exit(-1);
    }
    *np = atoi(argv[1]);
    *mp = atoi(argv[2]);
}