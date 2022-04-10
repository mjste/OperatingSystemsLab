#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <unistd.h>

#define BUFFER_SIZE 256
#define MAX_VARIABLE_COUNT 128

void function(int i, char components[MAX_VARIABLE_COUNT][BUFFER_SIZE], char commands[MAX_VARIABLE_COUNT][BUFFER_SIZE]);

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Incorrect argument number\n");
        exit(1);
    }
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        fprintf(stderr, "File can't be opened\n");
        exit(2);
    }

    // save components to array
    char components[MAX_VARIABLE_COUNT][BUFFER_SIZE];
    int index = 0;
    // int complen = 0;
    while (1)
    {
        if (index >= MAX_VARIABLE_COUNT)
        {
            fprintf(stderr, "Too many elements\n");
            exit(3);
        }
        if (fgets(components[index], BUFFER_SIZE, fp) == NULL)
            break;
        if (strcmp(components[index], "\n") == 0)
            break;

        index++;
    }
    // complen = index;

    // save commands to array
    char commands[MAX_VARIABLE_COUNT][BUFFER_SIZE];
    index = 0;
    int commlen = 0;
    while (1)
    {
        if (index >= MAX_VARIABLE_COUNT)
        {
            fprintf(stderr, "Too many elements\n");
            exit(3);
        }
        if (fgets(commands[index], BUFFER_SIZE, fp) == NULL)
            break;
        if (strcmp(commands[index], "\n\0") == 0)
            break;
        index++;
    }
    commlen = index;

    for (int i = 0; i < commlen; i++)
    {
        if (fork() == 0)
        {
            function(i, components, commands);
            exit(0);
        }
    }

    for (int i = 0; i < commlen; i++)
        wait(NULL);

    return 0;
}

void function(int index, char components[MAX_VARIABLE_COUNT][BUFFER_SIZE], char commands[MAX_VARIABLE_COUNT][BUFFER_SIZE])
{
    sleep(index);
    int cmds[BUFFER_SIZE];
    char *arg;
    arg = strtok(commands[index], " ");

    arg += 8;
    // printf("%s\n", arg);
    cmds[0] = atoi(arg);
    int cmdslen = 1;
    while ((arg = strtok(NULL, " ")))
    {
        if (arg[0] == '#')
            break;

        if (arg[0] != '|')
        {
            arg += 8;
            // printf("%s\n", arg);
            cmds[cmdslen++] = atoi(arg);
        }
    }

    for (int j = 0; j < cmdslen; j++)
    {
        printf("%d ", cmds[j]);
    }

    printf("\n");
}