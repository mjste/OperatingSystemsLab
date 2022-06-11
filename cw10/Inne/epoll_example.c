#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define READ_SIZE 50
#define MAX_EVENTS 5

int main()
{
    int running = 1, event_count, i;
    size_t bytes_read;
    char read_buffer[READ_SIZE + 1];
    struct epoll_event event, events[MAX_EVENTS];

    int epoll_fd = epoll_create1(0);

    if (epoll_fd == -1)
    {
        perror("epoll create");
        exit(0);
    }

    event.events = EPOLLIN;
    event.data.fd = 0;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, 0, &event))
    {
        perror("failed");
        close(epoll_fd);
        exit(0);
    }

    while (running)
    {
        printf("\nPolling for input...\n");
        event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 30000);
        printf("%d ready events\n", event_count);
        for (i = 0; i < event_count; i++)
        {
            printf("Reading file descriptor '%d' -- ", events[i].data.fd);
            bytes_read = read(events[i].data.fd, read_buffer, READ_SIZE);
            printf("%zd bytes read.\n", bytes_read);
            read_buffer[bytes_read] = 0;
            printf("Read '%s'\n", read_buffer);

            if (!strncmp(read_buffer, "stop\n", 5))
                running = 0;
        }
    }

    if (close(epoll_fd))
    {
        perror("close");
        exit(0);
    }
    return 0;
}