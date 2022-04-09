#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

int main()
{
    mkfifo("potok", S_IFIFO);
    int fd = open("potok", O_WRONLY);
    char text[] = "kbafkjnafjk\nafbakf,\nafjbajfk\n";
    write(fd, text, strlen(text));
    return 0;
}
