#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <pthread.h>
#include <string.h>
#include <sys/times.h>
#include <unistd.h>

enum p_variant
{
    NUMBERS,
    BLOCK
};

struct image
{
    int rows;
    int cols;
    unsigned char **array;
};

struct thread_argument
{
    struct image *img_in;
    struct image *img_out;
    enum p_variant variant;
    int from;
    int to;
};

void process_arguments(char **argv, int *no_of_threads, int *p_var);
void *thread_handler(void *args);

int main(int argc, char **argv)
{
    /*
    arguments:
    number of threads
    type: NUMBERS / BLOCK
    fin
    fout
    */
    int number_of_threads;
    int program_variant;

    if (argc != 5)
    {
        fprintf(stderr, "Usage: ./zad <threads_no> <type> <fin> <fout>\n");
        exit(0);
    }
    process_arguments(argv, &number_of_threads, &program_variant);

    struct image img1;
    struct image img2;

    FILE *fin = fopen(argv[3], "r");
    FILE *fout = fopen(argv[4], "w");

    char img_mode[10];
    int rows;
    int cols;
    int max_val;
    fscanf(fin, "%s %d %d %d", img_mode, &rows, &cols, &max_val);
    img1.cols = img2.cols = cols;
    img1.rows = img2.rows = rows;

    // allocate memory
    img1.array = malloc(rows * sizeof(char *));
    img2.array = malloc(rows * sizeof(char *));
    for (int i = 0; i < rows; i++)
    {
        img1.array[i] = malloc(cols * sizeof(char));
        img2.array[i] = malloc(cols * sizeof(char));
    }

    // load image into memory
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            fscanf(fin, "%d", (int *)&img1.array[i][j]);
    // close fin
    fclose(fin);

    // create_arguments
    int range_min = 0;
    int range_max;
    switch (program_variant)
    {
    case NUMBERS:
        range_max = 256;
        break;
    case BLOCK:
        range_max = rows * cols;
        break;
    }

    struct thread_argument **args = malloc(number_of_threads * sizeof(struct thread_argument *));
    for (int i = 0; i < number_of_threads; i++)
    {
        args[i] = malloc(sizeof(struct thread_argument));
        args[i]->img_in = &img1;
        args[i]->img_out = &img2;
        args[i]->variant = program_variant;
        args[i]->from = range_min + (range_max - range_min) * i / number_of_threads;
        args[i]->to = range_min + (range_max - range_min) * (i + 1) / number_of_threads;
    }

    // create threads
    pthread_t *threadsp = malloc(number_of_threads * sizeof(pthread_t));
    for (int i = 0; i < number_of_threads; i++)
    {
        if (pthread_create(&threadsp[i], NULL, &thread_handler, args[i]) != 0)
        {
            perror("Creating threads");
            exit(0);
        }
    }

    // wait for threads
    double *time_arr = malloc(number_of_threads * sizeof(double));
    for (int i = 0; i < number_of_threads; i++)
    {
        double *timep;
        pthread_join(threadsp[i], (void **)&timep);
        time_arr[i] = *timep;
    }
    puts("Threads finished");

    for (int i = 0; i < number_of_threads; i++)
    {
        printf("%f \n", time_arr[i]);
    }

    // save image
    fprintf(fout, "%s\n%d %d\n%d\n", img_mode, rows, cols, max_val);
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            fprintf(fout, "%d ", (unsigned char)img2.array[i][j]);
        }
        fprintf(fout, "\n");
    }

    // free memory
    for (int i = 0; i < rows; i++)
    {
        free(img1.array[i]);
        free(img2.array[i]);
    }
    free(img1.array);
    free(img2.array);

    fclose(fout);

    return 0;
}

void process_arguments(char **argv, int *no_of_threads, int *p_var)
{
    /*
    arguments:
    number of threads
    type: NUMBERS / BLOCK
    fin
    fout
    */

    //  no_of_threads
    if (!isdigit(argv[1][0]))
    {
        fprintf(stderr, "First argument is not a number\n");
        exit(-1);
    }
    *no_of_threads = atoi(argv[1]);

    // p_var
    if (strcmp(argv[2], "numbers") == 0)
    {
        *p_var = NUMBERS;
    }
    else if (strcmp(argv[2], "block") == 0)
    {
        *p_var = BLOCK;
    }
    else
    {
        fprintf(stderr, "error: bad mode, try 'numbers' or 'block'\n");
        exit(-1);
    }

    // fin
    FILE *fp = fopen(argv[3], "r");
    if (fp == NULL)
    {
        perror("fin can't be opened");
        exit(-1);
    }
    fclose(fp);

    // fout
    fp = fopen(argv[4], "w");
    if (fp == NULL)
    {
        perror("fout can't be opened");
        exit(-1);
    }
    fclose(fp);
    return;
}

void *thread_handler(void *args)
{
    time_t start = times(NULL);
    time_t stop;
    struct thread_argument *arg = args;
    switch (arg->variant)
    {
    case NUMBERS:
        for (int i = 0; i < arg->img_in->rows; i++)
        {
            for (int j = 0; j < arg->img_in->cols; j++)
            {
                unsigned char value = arg->img_in->array[i][j];
                if (value >= arg->from && value < arg->to)
                {
                    arg->img_out->array[i][j] = 255 - value;
                }
            }
        }
        break;
    case BLOCK:
    {
        int i = arg->from;
        while (i < arg->to)
        {
            int row = i / arg->img_in->cols;
            int col = i % arg->img_in->cols;
            arg->img_out->array[row][col] = 255 - arg->img_in->array[row][col];
            i++;
        }
        break;
    }
    }
    stop = times(NULL);
    double *running_time = malloc(sizeof(double));
    *running_time = ((double)stop - (double)start) / (double)sysconf(_SC_CLK_TCK);
    return running_time;
}