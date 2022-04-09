#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <ftw.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <wait.h>
#include <libgen.h>

void dirFunction(char *absolute_path, char *relative_path, int depth);
void fileFunction(const char *absolute_path, const char *relative_path, const struct stat *statp);
void generatePi(char *pattern, int *pi);
int kmp(char *text, int textsize, char *pattern, int *pi);

int *pi;
long total_children = 0;
char *pattern;
int pattern_length;
int maxlevel = 1;

int main(int argc, char **argv)
{
    /*
    arg1: directory
    arg2: pattern
    arg3: max_depth
    */
    if (argc != 4)
    {
        fprintf(stderr, "main: incorrect argument number\n");
        exit(1);
    }

    struct stat stat;
    lstat(argv[1], &stat);
    if (!S_ISDIR(stat.st_mode))
    {
        fprintf(stderr, "main: please enter a correct directory name first\n");
        exit(2);
    }

    if (!isdigit(argv[3][0]))
    {
        fprintf(stderr, "main: please enter correct depth\n");
        exit(2);
    }
    maxlevel = atoi(argv[3]);
    pattern = argv[2];
    pattern_length = strlen(pattern);
    pi = calloc(pattern_length, sizeof(int));
    generatePi(argv[2], pi);

    char abs_path[PATH_MAX];
    realpath(argv[1], abs_path);

    dirFunction(abs_path, argv[1], 1);

    free(pi);

    printf("Done\n");
    return 0;
}

// Prefix function for KMP algorithm
void generatePi(char *pattern, int *pi)
{
    int size = strlen(pattern);
    pi[0] = 0;
    int k = 0;
    for (int q = 1; q < size; q++)
    {
        while (k > 0 && pattern[k] != pattern[q])
        {
            k = pi[k - 1];
        }
        if (pattern[k] == pattern[q])
        {
            k += 1;
        }
        pi[q] = k;
    }
}

int kmp(char *text, int textsize, char *pattern, int *pi)
{
    int q = 0;
    int len = strlen(pattern);
    for (int i = 0; i < textsize; i++)
    {
        while (q > 0 && pattern[q] != text[i])
            q = pi[q - 1];
        if (pattern[q] == text[i])
            q++;
        if (q == len)
            return 1;
    }
    return 0;
}

void dirFunction(char *absolute_path, char *relative_path, int depth)
{
    if (depth > maxlevel)
        return;

    chdir(absolute_path);
    DIR *dp = opendir(".");
    if (dp == NULL)
    {
        return;
    }

    struct dirent *direntp;
    int children_count = 0;
    while ((direntp = readdir(dp)))
    {
        char new_absolute_path[PATH_MAX];
        char new_relative_path[PATH_MAX];
        strcpy(new_absolute_path, absolute_path);
        strcpy(new_relative_path, relative_path);
        strcat(new_absolute_path, "/");
        strcat(new_relative_path, "/");
        strcat(new_absolute_path, direntp->d_name);
        strcat(new_relative_path, direntp->d_name);
        struct stat stat;

        lstat(new_absolute_path, &stat);
        if (S_ISDIR(stat.st_mode))
        {
            if (strcmp(direntp->d_name, ".") != 0 && strcmp(direntp->d_name, "..") != 0)
            {
                children_count++;
                pid_t child_pid = fork();
                if (child_pid == 0)
                {
                    dirFunction(new_absolute_path, new_relative_path, depth + 1);
                    exit(0);
                }
            }
        }
        else if (S_ISREG(stat.st_mode))
        {
            char *ext = direntp->d_name; 
            int n = strlen(ext);
            ext += n-4;
            // check if file extension is .txt
            if (strcmp(ext, ".txt") == 0) {
                fileFunction(new_absolute_path, new_relative_path, &stat);
            }
        }
    }
    closedir(dp);

    for (int i = 0; i < children_count; i++)
    {
        wait(0);
    }

    return;
}

void fileFunction(const char *absolute_path, const char *relative_path, const struct stat *sb)
{
    // check if it is file that can be opened
    FILE *fp = fopen(absolute_path, "r");
    if (fp == NULL)
        return;

    int n = 100;
    int size = n * pattern_length;
    char *buffer = calloc(size + 1, 1);

    int bytes;
    while ((bytes = (int)fread(buffer, 1, size, fp)))
    {
        // clear buffer
        for (int i = bytes; i < size; i++)
            buffer[i] = 0;

        if (kmp(buffer, size, pattern, pi))
        {
            printf("path: %s\tpid: %ld\n", relative_path, (long int)getpid());
            break;
            free(buffer);
            fclose(fp);
            return;
        }

        if (bytes < pattern_length)
        {
            break;
        }
        else if (bytes > (n - 1) * pattern_length)
        {
            fseek(fp, -(long)pattern_length, SEEK_CUR);
        }
    }

    free(buffer);
    fclose(fp);
    return;
}