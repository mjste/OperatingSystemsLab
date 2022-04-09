#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ftw.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <limits.h>

int fileCount = 0;
int dirCount = 0;
int fifoCount = 0;
int blockCount = 0;
int charCount = 0;
int sockCount = 0;
int linkCount = 0;

int fileFunction(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);

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
    nftw(path, fileFunction, 10, FTW_PHYS);

    printf("\nTotal regular files: %d\n", fileCount);
    printf("Total directories: %d\n", dirCount);
    printf("Total block devices: %d\n", blockCount);
    printf("Total character devices: %d\n", charCount);
    printf("Total fifos: %d\n", fifoCount);
    printf("Total sockets: %d\n", sockCount);
    printf("Total symlinks: %d\n", linkCount);

    return 0;   
}


int fileFunction(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    // wyprintuj realpath
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