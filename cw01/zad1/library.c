#include "library.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BlockTable* createBlockTable(int size) {
    BlockTable* btp = calloc(1, sizeof(BlockTable));
    btp->size = size;
    btp->index = 0;
    btp->strings = calloc(size, sizeof(char**));
    return btp;
}

void wcFiles(int size, char** filenames) {
    int max_filename = 200;
    char* buffer = calloc((max_filename+1)*size,1);
    strcat(buffer, "wc ");
    
    for (int i = 0; i < size; i++) {
        if (strlen(filenames[i]) > max_filename)
             exit(1);
 
        strcat(buffer, filenames[i]);
        strcat(buffer, " ");
    }

    strcat(buffer, "> tmp");
    system(buffer);
    free(buffer);
}

int loadTmpFile(BlockTable* btp) {
    if (btp == NULL)
        return -1;
    if (btp->index >= btp->size)
        return -2;
    FILE* fp = fopen("tmp", "r");
    if (fp == NULL)
        return -3;
    fseek(fp, 0, SEEK_END);

    btp->strings[btp->index] = calloc(ftell(fp)+2, 1);

    // find first free index on right
    int index = btp->index;
    while (btp->strings[btp->index] != NULL && btp->index < btp->size) {
        btp->index++;
    }
    return index;
}


int freeBlock(BlockTable* btp, int index) {
    if (btp == NULL)
        return 1;
    if (index < 0 || index >= btp->size)
        return 2;
    char* cp = btp->strings[index];
    if (cp == NULL)
        return 3;
    free(cp);
    btp->strings[index] = NULL;
    if (index < btp->index)
        btp->index = index;
    return 0;
}


void freeBlockTable(BlockTable* btp) {
    // printf("starting to free\n");
    for (int i = 0; i < btp->size; i++) {
        freeBlock(btp, i);
    }
    // printf("Freed strings\n");
    free(btp->strings);
    btp->strings = NULL;
    free(btp);
    btp = NULL;
}