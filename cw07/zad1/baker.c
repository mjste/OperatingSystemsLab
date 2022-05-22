
#include "common.h"

void set_interrupt();
void int_handler(int signum);
double get_timestamp();
void wait_for_sem(int sem_set_id, int sem_no);
void free_sem(int sem_set_id, int sem_no);

int running = 1;
time_t start_time;

int main()
{
    start_time = times(NULL);
    srand(time(NULL));
    set_interrupt();
    // generate keys
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

    // open semaphores and memory
    int sem_set_id = semget(sem_set_key, 2, 0);
    if (sem_set_id == -1)
    {
        perror("sem_set_id");
        exit(-1);
    }
    int shm_oven_id = shmget(mem_oven_key, sizeof(struct container), 0);
    if (shm_oven_id == -1)
    {
        perror("shm_oven_id");
        exit(-1);
    }
    int shm_table_id = shmget(mem_table_key, sizeof(struct container), 0);
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
    struct container *shm_table_address = (struct container *)shmat(shm_table_id, NULL, 0);
    if (shm_table_address == (struct containter *)(-1))
    {
        perror("shm_table_at");
        exit(-1);
    }

    int pizza_number;
    int pizza_was_put_in_oven = 0;
    int pizza_was_put_on_table = 0;
    int oven_index;

    while (running)
    {
        // picking pizza number
        pizza_number = rand() % 10;
        pizza_was_put_in_oven = 0;
        printf("%d %.3f Przygotowuję pizze: %d\n", (int)getpid(), get_timestamp(), pizza_number);
        sleep(1);

        // put pizza in oven
        while (!pizza_was_put_in_oven && running)
        {
            wait_for_sem(sem_set_id, OVEN);

            if (shm_oven_address->items < 5)
            {
                shm_oven_address->items += 1;
                shm_oven_address->space[shm_oven_address->write_index] = pizza_number;
                oven_index = shm_oven_address->write_index;
                shm_oven_address->write_index = (oven_index + 1) % 5;
                printf("%d %.3f Dodałem pizze: %d, liczba pizz w piecu: %d\n", (int)getpid(), get_timestamp(), pizza_number, shm_oven_address->items);
                pizza_was_put_in_oven = 1;
            }

            free_sem(sem_set_id, OVEN);
            if (!pizza_was_put_in_oven)
                sleep(1);
        }
        sleep(4);

        // put pizza out of oven
        wait_for_sem(sem_set_key, OVEN);
        shm_oven_address->items -= 1;
        oven_index = shm_oven_address->read_index;
        shm_oven_address->read_index = (oven_index + 1) % 5;
        pizza_number = shm_oven_address->space[oven_index];
        free_sem(sem_set_id, OVEN);

        while (!pizza_was_put_on_table && running)
        {
            wait_for_sem(sem_set_id, TABLE);
            free_sem(sem_set_id, TABLE);
        }
    }

    if (shmdt(shm_oven_address) == -1)
    {
        perror("shm_oven_dt");
        exit(-1);
    }
    return 0;
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

// gettimeofday??
double get_timestamp()
{
    clock_t cycles = times(NULL);
    return (double)(cycles - start_time) / sysconf(_SC_CLK_TCK);
}

void wait_for_sem(int sem_set_id, int sem_no)
{
    struct sembuf sem_buf;
    sem_buf.sem_num = sem_no;
    sem_buf.sem_op = -1;
    sem_buf.sem_flg = 0;
    semop(sem_set_id, &sem_buf, 1); // wait for access
}

void free_sem(int sem_set_id, int sem_no)
{
    struct sembuf sem_buf;
    sem_buf.sem_num = sem_no;
    sem_buf.sem_op = 1;
    sem_buf.sem_flg = 0;
    semop(sem_set_id, &sem_buf, 1);
}