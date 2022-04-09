#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 2048

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

    FILE* f1 = fopen(name1, "r");
    if (!f1) {
        printf("error: zla nazwa pliku\n");
        exit(1);
    }
    FILE* f2 = fopen(name2, "w");


    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, f1) != NULL) {
        int i = 0;
        char not_empty = 0;
        while (buffer[i] != 0) {
            if (!isspace(buffer[i])) {
                not_empty = 1;
                break;
            }
            i++;
        }
        if (not_empty) {
            fputs(buffer, f2);
        }
    }


    fclose(f1);
    fclose(f2);
    printf("End\n");
    return 0;
}