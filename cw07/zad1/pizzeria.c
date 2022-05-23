#include "common.h"

void process_arguments(char **argv, int *n, int *m);
void exit0();
void set_interrupt();
void int_handler(int signum);

int running = 1;

int main(int argc, char **argv)
{
    /*
    arg1: n - number of bakers
    arg2: m - number of deliverers
    */
    int n, m;
    if (argc != 3)
    {
        printf("Usage: pizzeria <n> <m>\n");
        exit(-1);
    }
    process_arguments(argv, &n, &m);
    set_interrupt();

    // create keys
    int sem_set_key = ftok(getenv("HOME"), 'a');
    if (sem_set_key == -1)
    {
        perror("sem_set_key");
        exit(-1);
    }
    int mem_oven_key = ftok(getenv("HOME"), 'b');
    if (mem_oven_key == -1)
    {
        perror("mem_oven_key");
        exit(-1);
    }
    int mem_table_key = ftok(getenv("HOME"), 'c');
    if (mem_table_key == -1)
    {
        perror("mem_table_key");
        exit(-1);
    }

    // get semaphore array and shared memory
    int sem_set_id = semget(sem_set_key, 2, IPC_CREAT | IPC_EXCL | 0600);
    if (sem_set_id == -1)
    {
        perror("sem_set_id");
        exit(-1);
    }
    int shm_oven_id = shmget(mem_oven_key, sizeof(struct container), IPC_CREAT | IPC_EXCL | 0600);
    if (shm_oven_id == -1)
    {
        perror("shm_oven_id");
        exit(-1);
    }
    int shm_table_id = shmget(mem_table_key, sizeof(struct container), IPC_CREAT | IPC_EXCL | 0600);
    if (shm_table_id == -1)
    {
        perror("shm_table_id");
        exit(-1);
    }

    // get memory address
    struct container *shm_oven_address = (struct container *)shmat(shm_oven_id, NULL, 0);
    if (shm_oven_address == (struct container *)(-1))
    {
        perror("shm_oven_at");
        exit(-1);
    }

    // set shared memory content
    shm_oven_address->items = 0;
    shm_oven_address->read_index = 0;
    shm_oven_address->write_index = 0;
    for (int i = 0; i < 5; i++)
    {
        shm_oven_address->space[i] = -1;
    }

    // set semaphores
    semctl(sem_set_id, 0, SETVAL, 1);
    semctl(sem_set_id, 1, SETVAL, 1);

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

    while (running)
    {
        sleep(1);
    }

    // detach memory segment
    if (shmdt(shm_oven_address) == -1)
    {
        perror("shm_oven_dt");
        exit(-1);
    }

    // remove shared memory segments
    if (shmctl(shm_oven_id, IPC_RMID, NULL) == -1)
    {
        perror("remove_shm_oven");
        exit(-1);
    }
    if (shmctl(shm_table_id, IPC_RMID, NULL) == -1)
    {
        perror("remove_shm_table");
        exit(-1);
    }
    // remove semaphores
    if (semctl(sem_set_id, 0, IPC_RMID) == -1)
    {
        perror("remove_sem_set");
        exit(-1);
    }
    return 0;
}

void process_arguments(char **argv, int *n, int *m)
{
    if (!isdigit(argv[1][0]))
    {
        printf("first argument is not a number");
        exit(-1);
    }

    if (!isdigit(argv[2][0]))
    {
        printf("second argument is not a number");
        exit(-1);
    }

    *n = atoi(argv[1]);
    *m = atoi(argv[2]);
    return;
}

void set_interrupt()
{
    struct sigaction siga;
    siga.sa_flags = 0;
    sigemptyset(&siga.sa_mask);
    siga.sa_handler = int_handler;
    sigaction(SIGINT, &siga, NULL);
}

void int_handler(int signum)
{
    running = 0;
}
