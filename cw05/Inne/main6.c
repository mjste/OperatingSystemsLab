#include <unistd.h>

int main()
{
    char *args[3] = {"cat", "/etc/passwd", NULL};
    execvp("cat", args);
    return 0;
}