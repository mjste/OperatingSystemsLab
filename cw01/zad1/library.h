#ifndef _LIBRARY_H
#define _LIBRARY_H

// typedef struct{
//     char* chars;
//     long size;
// } String;

typedef struct BlockTable_{
    char** strings;
    int size;
    int index;
} BlockTable;

BlockTable* createBlockTable(int size);
void freeBlockTable(BlockTable* btp);
void wcFiles(int size, char** filenames);
int loadTmpFile(BlockTable* btp);
int freeBlock(BlockTable* btp, int index);

#endif
