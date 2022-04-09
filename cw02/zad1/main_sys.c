#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 2048
#define BLOCK_SIZE 128

void copyFragment(int f1, int f2, long offset1, long offset2) {
    int blocks = (offset2-offset1)/BLOCK_SIZE;
    int leftover = (offset2-offset1)%BLOCK_SIZE;
    char buffer[BLOCK_SIZE];

    lseek(f1, offset1, SEEK_SET);
    for (int i = 0; i < blocks; i++) {
        read(f1, buffer, BLOCK_SIZE);
        write(f2, buffer, BUFFER_SIZE);
    }
    read(f1, buffer, leftover);
    write(f2, buffer, leftover);
}

int main(int argc, char** argv) {

    char name1[200];
    char name2[200];

    switch (argc){
        case 2:
            strcpy(name1, argv[1]);
            printf("Enter second filename: ");
            scanf("%s", name2);
            break;
        case 3:
            strcpy(name1, argv[1]);
            strcpy(name2, argv[2]);
            break;
        default:
            printf("Enter first filename: ");
            scanf("%s", name1);
            printf("Enter second filename: ");
            scanf("%s", name2);
            break;
    }

    int f1 = open(name1, O_RDONLY);
    if (f1 < 0)
    {
        fprintf(stderr, "File %s does not exist or can't be accessed", name1);
        exit(1);
    }
    int f2 = open(name2, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (f2 < 0) {
        fprintf(stderr, "File %s can't be accessed", name2);
        exit(2);
    }


    printf("Start\n");
    
    char buffer[BUFFER_SIZE];
    char not_white = 0;
    int read_bytes = 0;
    long string_start_offset = 0;
    long string_stop_offset = 0;
    long current_offset = 0;
    long next_offset;

    while ((read_bytes = read(f1, buffer, BUFFER_SIZE))) {
        next_offset = lseek(f1, 0, SEEK_CUR);

        for (long i = 0; i < read_bytes; i++) {
            if (!isspace(buffer[i]))
                not_white = 1;
            if (buffer[i] == '\n') {
                string_stop_offset = current_offset+i+1;
                if (not_white) {
                    copyFragment(f1, f2, string_start_offset, string_stop_offset);
                }
                not_white = 0;
                string_start_offset = string_stop_offset;
            }
        }
        lseek(f1, next_offset, SEEK_SET);
        current_offset = next_offset;
    }

    printf("Done\n");
    return 0;
}
