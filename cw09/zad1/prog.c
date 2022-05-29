#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <errno.h>
#include "utils.h"
#include <unistd.h>

void *santa_function(void *argp);
void *elf_function(void *argp);
void *reindeer_function(void *argp);

sem_t elf_sem;
pthread_t elf_ids[3];
int elf_count = 0;
int reindeer_count = 0;
int elf_event_var = 0;
int reindeer_event_var = 0;
int event_var = 0;
int santa_elf_start_var = 0;
int elf_santa_start_var = 0;
int santa_elf_end_var = 0;
int elf_santa_end_var = 0;
int santa_reindeer_start_var = 0;
int reindeer_santa_start_var = 0;
int santa_reindeer_end_var = 0;
int reindeer_santa_end_var = 0;

pthread_mutex_t elf_count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reindeer_count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t santa_elf_start_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t elf_santa_start_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t santa_elf_end_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t elf_santa_end_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t santa_reindeer_start_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reindeer_santa_start_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t santa_reindeer_end_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reindeer_santa_end_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t elf_event_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reindeer_event_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t event_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t santa_elf_start_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t elf_santa_start_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_elf_end_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t elf_santa_end_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_reindeer_start_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t reindeer_santa_start_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_reindeer_end_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t reindeer_santa_end_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t event_cond = PTHREAD_COND_INITIALIZER;

int main()
{
    pthread_t santa_thread;
    pthread_t elf_threads[10];
    pthread_t reindeer_threads[9];

    if (sem_init(&elf_sem, 0, 3) == -1)
    {
        perror("elf_sem_init");
        exit(-1);
    }

    // create threads
    if (pthread_create(&santa_thread, NULL, &santa_function, NULL) != 0)
    {
        perror("create Santa");
        exit(-1);
    }
    for (int i = 0; i < 10; i++)
        if (pthread_create(&elf_threads[i], NULL, &elf_function, NULL) != 0)
        {
            perror("create elf");
            exit(-1);
        }
    for (int i = 0; i < 9; i++)
        if (pthread_create(&reindeer_threads[i], NULL, &reindeer_function, NULL) != 0)
        {
            perror("create reindeer");
            exit(-1);
        }

    pthread_join(santa_thread, NULL);
    for (int i = 0; i < 10; i++)
        pthread_join(elf_threads[i], NULL);

    for (int i = 0; i < 9; i++)
        pthread_join(reindeer_threads[i], NULL);

    sem_destroy(&elf_sem);
    return 0;
}

void *santa_function(void *argp)
{
    while (1)
    {
        pthread_mutex_lock(&event_mutex);
        if (event_var != 1)
        {
            pthread_cond_wait(&event_cond, &event_mutex);
            printf("Mikołaj: budzę się\n");
        }
        event_var = 0;

        // reindeers
        pthread_mutex_lock(&reindeer_event_mutex);
        if (reindeer_event_var == 1)
        {
            reindeer_event_var = 0;
            // zero reindeer_count
            pthread_mutex_lock(&reindeer_count_mutex);
            reindeer_count = 0;
            pthread_mutex_unlock(&reindeer_count_mutex);

            // inform reindeers of delivery of gifts
            pthread_mutex_lock(&santa_reindeer_start_mutex);
            santa_reindeer_start_var = 1;
            printf("Mikołaj: dostarczam zabawki\n");
            pthread_cond_broadcast(&santa_reindeer_start_cond);
            pthread_mutex_unlock(&santa_reindeer_start_mutex);

            // deliver the gifts
            sleep_range(2, 4);

            // wait for reindeers response
            pthread_mutex_lock(&reindeer_santa_start_mutex);
            if (reindeer_santa_start_var != 9)
            {
                pthread_cond_wait(&reindeer_santa_start_cond, &reindeer_santa_start_mutex);
            }
            reindeer_santa_start_var = 0;
            pthread_mutex_unlock(&reindeer_santa_start_mutex);

            // reset flag
            pthread_mutex_lock(&santa_reindeer_start_mutex);
            santa_reindeer_start_var = 0;
            pthread_mutex_unlock(&santa_reindeer_start_mutex);

            // inform reindeers of end of delivery
            pthread_mutex_lock(&santa_reindeer_end_mutex);
            santa_reindeer_end_var = 1;
            pthread_cond_broadcast(&santa_reindeer_end_cond);
            pthread_mutex_unlock(&santa_reindeer_end_mutex);

            // wait for elves' response
            pthread_mutex_lock(&reindeer_santa_end_mutex);
            if (reindeer_santa_end_var != 3)
            {
                pthread_cond_wait(&reindeer_santa_end_cond, &reindeer_santa_end_mutex);
            }
            reindeer_santa_end_var = 0;
            pthread_mutex_unlock(&reindeer_santa_end_mutex);

            // reset flag
            pthread_mutex_lock(&santa_reindeer_end_mutex);
            santa_reindeer_end_var = 0;
            pthread_mutex_unlock(&santa_reindeer_end_mutex);
        }
        pthread_mutex_unlock(&reindeer_event_mutex);

        // elves
        pthread_mutex_lock(&elf_event_mutex);
        if (elf_event_var == 1)
        {
            elf_event_var = 0;
            // zero elf_count
            pthread_mutex_lock(&elf_count_mutex);
            elf_count = 0;
            pthread_mutex_unlock(&elf_count_mutex);

            // inform elves of start of problem solving
            pthread_mutex_lock(&santa_elf_start_mutex);
            santa_elf_start_var = 1;
            printf("Mikołaj: rozwiązuje problemy elfów %lu %lu %lu\n", elf_ids[0], elf_ids[1], elf_ids[2]);
            pthread_cond_broadcast(&santa_elf_start_cond);
            pthread_mutex_unlock(&santa_elf_start_mutex);

            // solve problem
            sleep_range(1, 2);

            // wait for elves response
            pthread_mutex_lock(&elf_santa_start_mutex);
            if (elf_santa_start_var != 3)
            {
                pthread_cond_wait(&elf_santa_start_cond, &elf_santa_start_mutex);
            }
            elf_santa_start_var = 0;
            pthread_mutex_unlock(&elf_santa_start_mutex);

            // reset flag
            pthread_mutex_lock(&santa_elf_start_mutex);
            santa_elf_start_var = 0;
            pthread_mutex_unlock(&santa_elf_start_mutex);

            // inform elves of end of solving problem
            pthread_mutex_lock(&santa_elf_end_mutex);
            santa_elf_end_var = 1;
            pthread_cond_broadcast(&santa_elf_end_cond);
            pthread_mutex_unlock(&santa_elf_end_mutex);

            // wait for elves' response
            pthread_mutex_lock(&elf_santa_end_mutex);
            if (elf_santa_end_var != 3)
            {
                pthread_cond_wait(&elf_santa_end_cond, &elf_santa_end_mutex);
            }
            elf_santa_end_var = 0;
            pthread_mutex_unlock(&elf_santa_end_mutex);

            // reset flag
            pthread_mutex_lock(&santa_elf_end_mutex);
            santa_elf_end_var = 0;
            pthread_mutex_unlock(&santa_elf_end_mutex);
        }
        pthread_mutex_unlock(&elf_event_mutex);

        printf("Mikołaj: zasypiam\n");
        pthread_mutex_unlock(&event_mutex);
    }
}
void *elf_function(void *argp)
{
    while (1)
    {
        sleep_range(2, 5);
        sem_wait(&elf_sem);
        pthread_mutex_lock(&elf_count_mutex);
        elf_ids[elf_count] = pthread_self();
        elf_count += 1;
        printf("Elf: Czeka %d elfów na Mikołaja, %lu\n", elf_count, pthread_self());
        if (elf_count == 3)
        {
            pthread_mutex_lock(&event_mutex);
            pthread_mutex_lock(&elf_event_mutex);
            elf_event_var = 1;
            event_var = 1;
            printf("Elf: Wybudzam Mikołaja, %lu\n", pthread_self());
            pthread_cond_broadcast(&event_cond);
            pthread_mutex_unlock(&elf_event_mutex);
            pthread_mutex_unlock(&event_mutex);
        }
        pthread_mutex_unlock(&elf_count_mutex);

        // wait for santa's response
        pthread_mutex_lock(&santa_elf_start_mutex);
        if (santa_elf_start_var != 1)
            pthread_cond_wait(&santa_elf_start_cond, &santa_elf_start_mutex);
        printf("Elf: Mikołaj rozwiązuje problem, %lu\n", pthread_self());

        pthread_mutex_unlock(&santa_elf_start_mutex);

        // notify santa of receiving start
        pthread_mutex_lock(&elf_santa_start_mutex);
        elf_santa_start_var += 1;
        if (elf_santa_start_var == 3)
        {
            pthread_cond_broadcast(&elf_santa_start_cond);
        }
        pthread_mutex_unlock(&elf_santa_start_mutex);

        // wait for problem solved
        pthread_mutex_lock(&santa_elf_end_mutex);
        if (santa_elf_end_var != 1)
            pthread_cond_wait(&santa_elf_end_cond, &santa_elf_end_mutex);
        pthread_mutex_unlock(&santa_elf_end_mutex);

        // inform santa that you received message
        pthread_mutex_lock(&elf_santa_end_mutex);
        elf_santa_end_var += 1;
        if (elf_santa_end_var == 3)
            pthread_cond_broadcast(&elf_santa_end_cond);
        pthread_mutex_unlock(&elf_santa_end_mutex);

        // release the semaphore
        sem_post(&elf_sem);
    }
    return NULL;
}
void *reindeer_function(void *argp)
{
    while (1)
    {
        sleep_range(5, 10);
        pthread_mutex_lock(&reindeer_count_mutex);
        reindeer_count += 1;
        printf("Renifer: czeka %d reniferów na Mikołaja, %lu\n", reindeer_count, pthread_self());
        if (reindeer_count == 9)
        {
            pthread_mutex_lock(&event_mutex);
            pthread_mutex_lock(&reindeer_event_mutex);
            reindeer_event_var = 1;
            event_var = 1;
            printf("Renifer: Wybudzam Mikołaja, %lu\n", pthread_self());
            pthread_cond_broadcast(&event_cond);
            pthread_mutex_unlock(&reindeer_event_mutex);
            pthread_mutex_unlock(&event_mutex);
        }
        pthread_mutex_unlock(&reindeer_count_mutex);

        // wait for santa's response
        pthread_mutex_lock(&santa_reindeer_start_mutex);
        if (santa_reindeer_start_var != 1)
            pthread_cond_wait(&santa_reindeer_start_cond, &santa_reindeer_start_mutex);

        pthread_mutex_unlock(&santa_reindeer_start_mutex);

        // notify santa of receiving start
        pthread_mutex_lock(&reindeer_santa_start_mutex);
        reindeer_santa_start_var += 1;
        if (reindeer_santa_start_var == 9)
        {
            pthread_cond_broadcast(&reindeer_santa_start_cond);
        }
        pthread_mutex_unlock(&reindeer_santa_start_mutex);

        // wait for problem solved
        pthread_mutex_lock(&santa_reindeer_end_mutex);
        if (santa_reindeer_end_var != 1)
            pthread_cond_wait(&santa_reindeer_end_cond, &santa_reindeer_end_mutex);
        pthread_mutex_unlock(&santa_reindeer_end_mutex);

        // inform santa that you received message
        pthread_mutex_lock(&reindeer_santa_end_mutex);
        reindeer_santa_end_var += 1;
        if (reindeer_santa_end_var == 9)
            pthread_cond_broadcast(&reindeer_santa_end_cond);
        pthread_mutex_unlock(&reindeer_santa_end_mutex);
    }
    return NULL;
}
