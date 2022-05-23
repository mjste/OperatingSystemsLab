#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <pthread.h>

enum p_variant
{
    numbers,
    block
};

struct image
{
    int rows;
    int cols;
    char **array;
};

void process_arguments(char **argv, int *no_of_threads, int *p_var);
void thread_function(void *p);

int main(int argc, char **argv)
{
    /*
    arguments:
    number of threads
    type: numbers / block
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

    // create_arguments
    int args[20]; // mock, nie korzystać

    // create threads
    pthread_t *threadsp = malloc(number_of_threads * sizeof(pthread_t));
    for (int i = 0; i < number_of_threads; i++)
    {
        if (pthread_create(&threadsp[i], NULL, thread_function, args[i]))
            ;
    }

    // reverse colors
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            img2.array[i][j] = 255 - img1.array[i][j];

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

    fclose(fin);
    fclose(fout);

    return 0;
}

void process_arguments(char **argv, int *no_of_threads, int *p_var)
{
    /*
    arguments:
    number of threads
    type: numbers / block
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

    // p_var = {0|1}
    if (!isdigit(argv[2][0]))
    {
        fprintf(stderr, "Second argument is not a number\n");
        exit(-1);
    }
    *p_var = atoi(argv[2]);
    if (*p_var > 1 || *p_var < 0)
    {
        fprintf(stderr, "error: bad mode, try 0 or 1\n");
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

void thread_function(void *p);