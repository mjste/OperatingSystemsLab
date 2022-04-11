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
    int complen = 0;
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
    complen = index;

    // process components
    for (int i = 0; i < complen; i++)
    {
        char *arg;
        arg = strtok(components[i], "=");
        arg = strtok(NULL, "\n");
        strcpy(components[i], arg + 1);
    }

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
        wait(NULL);
    }

    // for (int i = 0; i < commlen; i++)
    //     wait(NULL);

    return 0;
}

void function(int index, char components[MAX_VARIABLE_COUNT][BUFFER_SIZE], char commands[MAX_VARIABLE_COUNT][BUFFER_SIZE])
{
    // sleep(index);
    int cmds[BUFFER_SIZE];
    char *arg;
    arg = strtok(commands[index], " ");

    // create component list
    arg += 8;
    cmds[0] = atoi(arg);
    int cmdslen = 1;
    while ((arg = strtok(NULL, " ")))
    {
        if (arg[0] == '#')
            break;

        if (arg[0] != '|')
        {
            arg += 8;
            cmds[cmdslen++] = atoi(arg);
        }
    }
    // Done

    // create one command
    char cmd[BUFFER_SIZE];
    cmd[0] = 0;
    for (int i = 0; i < cmdslen; i++)
    {
        strcat(cmd, components[cmds[i] - 1]);
        strcat(cmd, " | ");
    }
    // null last space
    cmd[strlen(cmd) - 1] = 0;
    // printf("One command: %s\n", cmd);
    printf("%s\n", cmd);

    // put command into subcommands
    char subcommands[MAX_VARIABLE_COUNT][BUFFER_SIZE];
    char *cmdn = cmd;
    arg = NULL;
    int subcmdn = 0;
    while ((arg = strtok(cmdn, "|")))
    {
        cmdn = NULL;
        if (arg[0] == ' ')
            arg++;
        strcpy(subcommands[subcmdn++], arg);
        // printf("%s\n", arg);
    }

    // printf("subcommands:\n");
    // for (int i = 0; i < subcmdn; i++)
    // {
    //     printf("%s\tlen: %ld\n", subcommands[i], strlen(subcommands[i]));
    // }
    // printf("\n");

    // create pipes
    int *pipes = calloc((subcmdn - 1) * 2, sizeof(int));
    for (int i = 0; i < subcmdn - 1; i++)
    {
        pipe(pipes + 2 * i);
    }

    // create process for each subcommand
    for (int i = 0; i < subcmdn; i++)
    {
        if (fork() == 0)
        {
            // printf("index: %d, i: %d\n", index, i);
            if (i == 0)
            {
                dup2(pipes[2 * i + 1], STDOUT_FILENO);
            }
            else if (i == subcmdn - 1)
            {
                dup2(pipes[2 * (i - 1)], STDIN_FILENO);
            }
            else
            {
                dup2(pipes[2 * (i - 1)], STDIN_FILENO);
                dup2(pipes[2 * i + 1], STDOUT_FILENO);
            }
            // close all other pipes
            for (int j = 0; j < 2 * (subcmdn - 1); j++)
            {
                if (j != 2 * (i - 1) && j != 2 * i + 1)
                {
                    close(pipes[j]);
                }
            }

            // create args for exec
            char **args = calloc(MAX_VARIABLE_COUNT, sizeof(char *));

            char *ptr = subcommands[i];
            char *initp = ptr;
            int ind = 0;
            while ((ptr = strtok(initp, " ")))
            {
                initp = NULL;
                args[ind] = calloc(BUFFER_SIZE, sizeof(char));
                strcpy(args[ind++], ptr);
                // printf("thissssssss %s\tlen: %ld\n", ptr, strlen(ptr));
            }
            args[ind] = NULL;

            // check args if they are containing ' or ", and remove it
            for (int j = 0; j < ind; j++)
            {
                if (args[j][0] == '\'')
                {
                    strcpy(args[j], &args[j][1]);
                    args[j][strlen(args[j]) - 1] = 0;
                    // printf("%s\n", args[j]);
                }
            }

            execvp(args[0], args);
            // printf("after exec\n\n");
            // printf("index: %d, i: %d\n", index, i);

            exit(0);
        }
        // wait(NULL);
    }

    // printf("\n\n");

    for (int i = 0; i < 2 * (subcmdn - 1); i++)
        close(pipes[i]);

    for (int i = 0; i < subcmdn; i++)
        wait(NULL);

    free(pipes);
}