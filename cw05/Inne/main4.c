#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

int main()
{
    int fp = open("potok", O_RDONLY);
    char text[100];
    read(fp, text, 100);
    printf("%s", text);
    return 0;
}
