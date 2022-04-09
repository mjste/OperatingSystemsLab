#define _DEFAULT_SOURCE

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <libgen.h>



int fileCount = 0;
int dirCount = -1;
int fifoCount = 0;
int blockCount = 0;
int charCount = 0;
int sockCount = 0;
int linkCount = 0;

int dirFunction(char* path);
int fileFunction(const char *fpath, const struct stat *sb);

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Incorrect number of arguments\n");
        exit(1);
    }

    struct stat stat;
    lstat(argv[1], &stat);
    if (!S_ISDIR(stat.st_mode)) {
        fprintf(stderr, "main_stat: No such directory\n");
        exit(1);
    }

    char path[PATH_MAX];
    realpath(argv[1], path);
    printf("path\thard_links\ttype\tbytes\taccess\tmodification\n\n");
    dirFunction(path);


    printf("\nTotal regular files: %d\n", fileCount);
    printf("Total directories: %d\n", dirCount);
    printf("Total block devices: %d\n", blockCount);
    printf("Total character devices: %d\n", charCount);
    printf("Total fifos: %d\n", fifoCount);
    printf("Total sockets: %d\n", sockCount);
    printf("Total symlinks: %d\n", linkCount);

    return 0;
}


int fileFunction(const char *fpath, const struct stat *sb) {
    printf("%s\t", fpath);
    printf("%d\t", (int)sb->st_nlink);
 
    if (S_ISDIR(sb->st_mode)) {
        dirCount++;
        printf("dir\t");
    } else if (S_ISREG(sb->st_mode)) {
        fileCount++;
        printf("file\t");
    } else if (S_ISCHR(sb->st_mode)) {
        charCount++;
        printf("char dev\t");
    } else if (S_ISBLK(sb->st_mode)) {
        blockCount++;
        printf("block dev\t");
    } else if (S_ISSOCK(sb->st_mode)) {
        sockCount++;
        printf("sock\t");
    } else if (S_ISLNK(sb->st_mode)) {
        linkCount++;
        printf("link\t");
    } else if (S_ISFIFO(sb->st_mode)) {
        fifoCount++;
        printf("fifo\t");
    }

    printf("%ld\t", (long)sb->st_size);
    char * ctime_char = strtok(ctime(&sb->st_atime) , "\n");
    printf("%s\t", ctime_char);
    ctime_char = strtok(ctime(&sb->st_mtime) , "\n");
    printf("%s\n", ctime_char);
    return 0;
}


int dirFunction(char* path) {
    chdir (path);

    struct stat stat;
    lstat(path, &stat);
    fileFunction(path, &stat);

    struct dirent *direntp;
    DIR* dp = opendir(".");
    if (dp != NULL) {
        char newpath[PATH_MAX];
        while ((direntp = readdir(dp)) != NULL) {
            strcpy(newpath, path);
            strcat(newpath, "/");
            strcat(newpath, direntp->d_name);
            // printf("%s\n", newpath);
            if (direntp->d_type == DT_DIR) {
                // printf("this is a directory!\n");
                if ((strcmp(direntp->d_name, ".") != 0) && (strcmp(direntp->d_name, "..") != 0)) {
                    dirFunction(newpath);
                }
            } else {
                struct stat filestat;
                lstat(direntp->d_name, &filestat);
                fileFunction(newpath, &filestat);
            }
        }
        closedir(dp);   
        return 0;  
    } else {
        fprintf(stderr, "error: %s can't be opened", path);
        return 1;
    }
}