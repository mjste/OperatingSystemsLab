#include "common.h"

void set_interrupt();
void int_handler(int signum);
double get_timestamp();

int running = 1;
struct timeval start_time;

int main()
{
    gettimeofday(&start_time, NULL);
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
    if (shm_table_address == (struct container *)(-1))
    {
        perror("shm_table_at");
        exit(-1);
    }

    int pizza_number;
    int pizza_was_put_in_oven = 0;
    int pizza_was_put_on_table = 0;
    int pizzas_in_oven;
    int oven_index;

    while (running)
    {
        // picking pizza number
        pizza_number = rand() % 10;
        printf("%d %.3f Przygotowuję pizze: %d\n", (int)getpid(), get_timestamp(), pizza_number);
        sleep(1);

        // put pizza in oven
        pizza_was_put_in_oven = 0;
        while (!pizza_was_put_in_oven && running)
        {
            wait_for_sem(sem_set_id, OVEN);

            if (shm_oven_address->items < 5)
            {
                shm_oven_address->items += 1;
                shm_oven_address->space[shm_oven_address->write_index] = pizza_number;
                oven_index = shm_oven_address->write_index;
                shm_oven_address->write_index = (oven_index + 1) % 5;
                printf("%d %.3f Dodałem pizze: %d, liczba pizz w piecu: %d\n",
                       (int)getpid(),
                       get_timestamp(),
                       pizza_number,
                       shm_oven_address->items);
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
        pizzas_in_oven = shm_oven_address->items;
        free_sem(sem_set_id, OVEN);

        // put pizza on table
        pizza_was_put_on_table = 0;
        while (!pizza_was_put_on_table && running)
        {
            wait_for_sem(sem_set_id, TABLE);

            if (shm_table_address->items < 5)
            {
                shm_table_address->items += 1;
                int index = shm_table_address->write_index;
                shm_table_address->write_index = (index + 1) % 5;
                shm_table_address->space[index] = pizza_number;
                printf("%d %.3f Wyjmuję pizzę: %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d\n",
                       (int)getpid(),
                       get_timestamp(),
                       pizza_number,
                       pizzas_in_oven,
                       shm_table_address->items);
                pizza_was_put_on_table = 1;
            }
            free_sem(sem_set_id, TABLE);
        }
    }

    if (shmdt(shm_oven_address) == -1)
    {
        perror("shm_oven_dt");
        exit(-1);
    }
    if (shmdt(shm_table_address) == -1)
    {
        perror("shm_table_dt");
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

double get_timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)(tv.tv_sec - start_time.tv_sec) + (tv.tv_usec - start_time.tv_usec) * 0.000001;
}