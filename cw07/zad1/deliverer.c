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
    int shm_table_id = shmget(mem_table_key, sizeof(struct container), 0);
    if (shm_table_id == -1)
    {
        perror("shm_table_id");
        exit(-1);
    }

    struct container *shm_table_address = (struct container *)shmat(shm_table_id, NULL, 0);
    if (shm_table_address == (struct container *)(-1))
    {
        perror("shm_table_at");
        exit(-1);
    }

    int pizza_number;
    int pizzas_on_table;
    int table_index;
    int pizza_taken_from_table = 0;

    while (running)
    {
        // get pizzas off table
        pizza_taken_from_table = 0;
        while (!pizza_taken_from_table && running)
        {
            wait_for_sem(sem_set_key, TABLE);
            if (shm_table_address->items > 0)
            {
                shm_table_address->items -= 1;
                table_index = shm_table_address->read_index;
                shm_table_address->read_index = (table_index + 1) % 5;
                pizza_number = shm_table_address->space[table_index];
                pizzas_on_table = shm_table_address->items;
                printf("%d %.3f Pobieram pizzę: %d. Liczba pizz na stole: %d\n",
                       (int)getpid(),
                       get_timestamp(),
                       pizza_number,
                       pizzas_on_table);
                pizza_taken_from_table = 1;
            }
            free_sem(sem_set_id, OVEN);
        }

        // go to client
        sleep_range(4, 5);
        printf("%d %.3f Dostarczam pizzę: %d.\n",
               (int)getpid(),
               get_timestamp(),
               pizza_number);
        sleep_range(4, 5);
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