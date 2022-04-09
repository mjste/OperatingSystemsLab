#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

int main(int argc, char** argv) {

    if (argc != 3) {
        fprintf(stderr, "Program needs 2 arguments: character and filename.\n");
        exit(1);
    }
    if (strlen(argv[1]) != 1) {
        fprintf(stderr, "First argument needst to be 1 character.\n");
        exit(2);
    }

    int fp = open(argv[2], O_RDONLY);
    if (fp < 0) {
        fprintf(stderr, "File %s does not exist or can't be accessed.\n", argv[2]);
        exit(3);
    }
    
    char buffer[BUFFER_SIZE];
    long lines_ok = 0;
    int chars_in_line = 0;
    int read_bytes = 0;
    long total_chars = 0;

    while ((read_bytes = read(fp, buffer, BUFFER_SIZE))) {
        for (int i = 0; i < read_bytes; i++) {
            if (buffer[i] == argv[1][0]) {
                chars_in_line = 1;
                total_chars++;
            }
                
            if (buffer[i] == '\n') {
                if (chars_in_line) {
                    lines_ok++;
                }
                chars_in_line = 0;
            }
        }
    }

    printf("Total count: %ld, lines count: %ld\n", total_chars, lines_ok);

    return 0;
}
