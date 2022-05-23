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
    sem_t *oven_access_sem = sem_open("/oven_access", 0);
    if (oven_access_sem == SEM_FAILED)
    {
        perror("oven_access_sem");
        exit(-1);
    }
    sem_t *oven_count_sem = sem_open("/oven_count", 0);
    if (oven_count_sem == SEM_FAILED)
    {
        perror("oven_count_sem");
        exit(-1);
    }
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
    int oven_mem_id = shm_open("/oven_mem", O_RDWR, 0);
    if (oven_mem_id == -1)
    {
        perror("oven_mem_id");
        exit(-1);
    }
    int table_mem_id = shm_open("/table_mem", O_RDWR, 0);
    if (table_mem_id == -1)
    {
        perror("table_mem_id");
        exit(-1);
    }

    // map shared memory
    struct container *ovenp = (struct container *)mmap(NULL, sizeof(struct container), PROT_READ | PROT_WRITE, MAP_SHARED, oven_mem_id, 0);
    if (ovenp == (struct container *)(-1))
    {
        perror("map_oven");
        exit(-1);
    }
    struct container *tablep = (struct container *)mmap(NULL, sizeof(struct container), PROT_READ | PROT_WRITE, MAP_SHARED, table_mem_id, 0);
    if (tablep == (struct container *)(-1))
    {
        perror("map_table");
        exit(-1);
    }

    // close memory descriptors
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

    int pizza_type;
    int index;
    int pizzas_in_oven;
    int pizzas_on_table;
    // run
    while (running)
    {
        // prepare pizza
        pizza_type = rand() % 10;
        printf("%d %.3f Przygotowuję pizzę: %d\n",
               getpid(),
               get_timestamp(), pizza_type);
        sleep_range(1, 2);

        // put pizza into oven
        sem_wait(oven_count_sem);
        sem_wait(oven_access_sem);

        ovenp->items += 1;
        index = ovenp->write_index;
        ovenp->space[index] = pizza_type;
        ovenp->write_index = (index + 1) % 5;
        printf("%d %.3f Dodałem pizzę: %d. Liczba pizz w piecu: %d.\n",
               getpid(),
               get_timestamp(),
               pizza_type,
               ovenp->items);
        sem_post(oven_access_sem);

        sleep_range(4, 5);
        // put pizza out of oven
        sem_wait(oven_access_sem);
        ovenp->items -= 1;
        pizzas_in_oven = ovenp->items;
        index = ovenp->read_index;
        pizza_type = ovenp->space[index];
        ovenp->read_index = (index + 1) % 5;

        sem_post(oven_access_sem);
        sem_post(oven_count_sem);

        sem_wait(table_count_sem);
        sem_wait(table_access_sem);
        // put pizza on table
        tablep->items += 1;
        pizzas_on_table = tablep->items;
        index = tablep->read_index;
        tablep->space[index] = pizza_type;
        tablep->write_index = (index + 1) % 5;

        printf("%d %.3f Wyjmuję pizzę: %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d\n",
               getpid(),
               get_timestamp(),
               pizza_type,
               pizzas_in_oven,
               pizzas_on_table);
        sem_post(table_access_sem);
    }

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

    // unmap
    if (munmap((void *)(ovenp), sizeof(struct container)) == -1)
    {
        perror("unmap_ovenp");
        exit(-1);
    }
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