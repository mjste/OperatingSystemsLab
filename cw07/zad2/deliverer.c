#include "common.h"

void sigint_handler(int signum);
double get_timestamp();

int running = 1;
struct timeval start_time;

int main()
{
    gettimeofday(&start_time, NULL);
    srand(time(NULL));
    set_sigint(sigint_handler);
    // open semaphores
    sem_t *table_access_sem = sem_open("/table_access", 0);
    if (table_access_sem == SEM_FAILED)
    {
        perror("table_access_sem");
        exit(-1);
    }
    sem_t *table_count_sem = sem_open("/table_count", 0);
    if (table_count_sem == SEM_FAILED)
    {
        perror("table_count_sem");
        exit(-1);
    }

    // open shared memory
    int table_mem_id = shm_open("/table_mem", O_RDWR, 0);
    if (table_mem_id == -1)
    {
        perror("table_mem_id");
        exit(-1);
    }

    // map shared memory
    struct container *tablep = (struct container *)mmap(NULL, sizeof(struct container), PROT_READ | PROT_WRITE, MAP_SHARED, table_mem_id, 0);
    if (tablep == (struct container *)(-1))
    {
        perror("map_table");
        exit(-1);
    }

    // close memory descriptors
    if (close(table_mem_id) == -1)
    {
        perror("closing_table_handle");
        exit(-1);
    }

    int pizza_type;
    int index;
    int pizzas_on_table;
    int pizza_taken;
    // run
    while (running)
    {
        // take pizza from table
        pizza_taken = 0;
        while (!pizza_taken)
        {
            sem_wait(table_access_sem);
            if (tablep->items > 0)
            {
                index = tablep->read_index;
                pizza_type = tablep->space[index];
                tablep->read_index = (index + 1) % 5;
                tablep->items -= 1;
                pizzas_on_table = tablep->items;
                pizza_taken = 1;
            }
            sem_post(table_access_sem);
            if (!pizza_taken)
                sleep_range(0, 1);
        }
        printf("%d %.3f Pobieram pizzę: %d. Liczba pizz na stole: %d.\n",
               getpid(),
               get_timestamp(),
               pizza_type,
               pizzas_on_table);
        sem_post(table_count_sem);

        // go to client
        sleep_range(4, 5);

        printf("%d %.4f Dostarczam pizzę: %d.\n",
               getpid(),
               get_timestamp(),
               pizza_type);

        // return from client
        sleep_range(4, 5);
    }

    // close semaphores
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

    // unmap
    if (munmap((void *)(tablep), sizeof(struct container)) == -1)
    {
        perror("unmap_tablep");
        exit(-1);
    }

    return 0;
}

void sigint_handler(int signum)
{
    running = 0;
}

double get_timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)(tv.tv_sec - start_time.tv_sec) + (tv.tv_usec - start_time.tv_usec) * 0.000001;
}