#include "../zad1/library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>


void start_clock();
void stop_clock();
double clk_to_sec(clock_t diff);
void print_time();

enum commands {
    EMPTY_COMMAND,
    CREATE_TABLE,
    REMOVE_TABLE,
    WC_FILES,
    REMOVE_BLOCK,
    LOAD_FILE,
    START_CLOCK,
    STOP_CLOCK,
    VERBOSE,
    PRINT, 
    ARGUMENT
};

enum error_numbers {
    UNKNOWN_ERROR,
    TABLE_NOT_CREATED,
    NEGATIVE_SIZE,
    TABLE_FULL
};

int mapString(char* str) {
    if(strcmp(str, "--create_table") == 0) {
        return CREATE_TABLE;
    }
    if(strcmp(str, "--wc_files") == 0) {
        return WC_FILES;
    }
    if(strcmp(str, "--remove_block") == 0) {
        return REMOVE_BLOCK;
    }
    if(strcmp(str, "--load_file") == 0) {
        return LOAD_FILE;
    }
    if(strcmp(str, "--start_clock") == 0) {
        return START_CLOCK;
    }
    if(strcmp(str, "--stop_clock") == 0) {
        return STOP_CLOCK;
    }
    if(strcmp(str, "--verbose") == 0) {
        return VERBOSE;
    }
    if(strcmp(str, "--print") == 0) {
        return PRINT;
    }
    if(strcmp(str, "--remove_table") == 0) {
        return REMOVE_TABLE;
    }
    return ARGUMENT;
}

clock_t time_start, time_stop;
struct tms tms_start, tms_stop;
char running = 0;

int main(int argc, char** argv) {
    char prev_command = EMPTY_COMMAND;
    int prev_command_index = 1;
    char verbose = 0;
    BlockTable* btp = NULL;
    for (int i = 1; i < argc; i++) {
        if (verbose)
            printf("Command: %s, code: %d\n", argv[i], mapString(argv[i]));
        switch (mapString(argv[i]))
        {
        case CREATE_TABLE:
            prev_command = CREATE_TABLE;
            break;
        case REMOVE_TABLE:
            prev_command = REMOVE_TABLE;
            if (btp != NULL) {
                freeBlockTable(btp);
                btp = NULL;
            }
            break;
        case WC_FILES:
            if (verbose)
                printf("wc_files: \n");
            prev_command = WC_FILES;
            prev_command_index = i;
            break;
        case REMOVE_BLOCK:
            prev_command = REMOVE_BLOCK;
            break;
        case LOAD_FILE:
            {
                prev_command = LOAD_FILE;
                if (btp == NULL)
                    exit(TABLE_NOT_CREATED);
                int index = loadTmpFile(btp);
                if (index < 0)
                    exit(UNKNOWN_ERROR);
                
                break;
            }
        case START_CLOCK:
            prev_command = START_CLOCK;
            start_clock();
            break;
        case STOP_CLOCK:
            prev_command = STOP_CLOCK;
            stop_clock();
            break;
        case VERBOSE:
            prev_command = VERBOSE;
            verbose = !verbose;
            break;
        case PRINT:
            prev_command = PRINT;
            break;
        case ARGUMENT:
            switch (prev_command)
            {
            case CREATE_TABLE:
            {
                int size = atoi(argv[i]);
                if (size <= 0)
                    exit(NEGATIVE_SIZE);
                btp = createBlockTable(size);
                if (verbose)
                    printf("Created table, size: %d\n", size);
                break;
            }
            case WC_FILES:
                // if (verbose) {
                //     printf("filename: %s\n", argv[i]);
                // }
                if (i+1 == argc || (mapString(argv[i+1]) != ARGUMENT)) {
                    if (verbose)
                        printf("Last file\n");
                    int size = i-prev_command_index;
                    char** filenames = argv+(prev_command_index+1);
                    wcFiles(size, filenames);
                    if (verbose)
                        printf("counted\n");
                }
                break;
            case REMOVE_BLOCK:
                {
                    int index = isdigit(argv[i][0]) ? atoi(argv[i]) : -1;
                    if (index < 0)
                        exit(NEGATIVE_SIZE);
                    
                    break;
                }
            case PRINT:
                printf("%s\n", argv[i]);
                break;
            }
            break;
        }
    }
    printf("Done\n");

}

void start_clock() {
    time_start = times(&tms_start);
    running = 1;
}
void stop_clock() {
    if (running) {
        running = 0;
        time_stop = times(&tms_stop);
        print_time();
    }
}
double clk_to_sec(clock_t diff) {
    return (double)(diff)/(double)(sysconf(_SC_CLK_TCK));
}
void print_time() {
    printf("Total time: %.3f, user time: %.3f, system time %.3f\n",
        clk_to_sec(time_stop-time_start),
        clk_to_sec(tms_stop.tms_cutime-tms_start.tms_cutime),
        clk_to_sec(tms_stop.tms_cstime-tms_start.tms_cstime));
}