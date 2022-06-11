#include "common.h"

#define MAX_EPOLL_EVENTS 2
#define BUFFER_SIZE 50
#define READ_SIZE (BUFFER_SIZE - 1)
#define LISTEN_NUM 10
#define MAX_CLIENTS 20

struct client_s
{
    int sfd;
    enum play_sign c_play_sign;
};

void process_input(int argc, char **argv, int *port_number, char *socket_path);
void handle_client(int sfd, struct client_s *clients);

int free_client_index = 0;
struct client_s clients[MAX_CLIENTS];
pthread_mutex_t free_client_index_mutex;
pthread_mutex_t clients_mutex;

int main(int argc, char **argv)
{
    char socket_path[PATH_MAX];
    int port_number;

    process_input(argc, argv, &port_number, socket_path);

    // two accepting sockets
    int local_sfd, inet_sfd, epoll_fd;
    int running = 1;
    int epoll_count;
    struct sockaddr_in saddr_in;
    struct sockaddr_un saddr_un;
    struct sockaddr saddr2;
    size_t bytes_read;
    char read_buffer[BUFFER_SIZE];
    struct epoll_event event, events[2];

    // creating sockets
    inet_sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (inet_sfd == -1)
    {
        perror("creating inet socket");
        return -1;
    }

    local_sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (local_sfd == -1)
    {
        perror("creating local socket");
        return -1;
    }

    // filling structures
    saddr_in.sin_family = AF_INET;
    saddr_in.sin_port = htons(port_number);
    saddr_in.sin_addr.s_addr = htonl(INADDR_ANY);

    saddr_un.sun_family = AF_UNIX;
    strcpy(saddr_un.sun_path, socket_path);

    // binding sockets
    if (bind(inet_sfd, (struct sockaddr *)&saddr_in, sizeof(saddr_in)) == -1)
    {
        perror("binding inet");
        return -1;
    }

    if (remove(SOCKET_PATH) == -1 && errno != ENOENT)
    {
        perror("remove");
        return -1;
    }

    if (bind(local_sfd, (struct sockaddr *)&saddr_un, sizeof(saddr_un)) == -1)
    {
        perror("binding local");
        return -1;
    }

    // epoll
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        perror("create epoll");
        return -1;
    }
    event.events = EPOLLIN;
    event.data.fd = inet_sfd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, inet_sfd, &event) == -1)
    {
        perror("epoll add inet fail");
        return -1;
    }

    event.events = EPOLLIN;
    event.data.fd = local_sfd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, local_sfd, &event) == -1)
    {
        perror("epoll add local fail");
        return -1;
    }
    // epoll1 done

    if (listen(inet_sfd, LISTEN_NUM) == -1)
    {
        perror("listen inet");
        return -1;
    }
    if (listen(local_sfd, LISTEN_NUM) == -1)
    {
        perror("listen local");
        return -1;
    }

    printf("Successfully opened server\n");

    while (running)
    {
        int epoll_count = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS, 30000);
        // printf("Hello there\n");
        for (int i = 0; i < epoll_count; i++)
        {
            int cur_fd = events[i].data.fd;

            // printf("%d\n", epoll_count);
            // if (cur_fd == inet_sfd)
            // {
            //     printf("Connected over inet\n");
            // }
            // else if (cur_fd == local_sfd)
            // {
            //     printf("Connected over local\n");
            // }
            // else
            // {
            //     printf("error\n");
            //     exit(-1);
            // }

            if (cur_fd == inet_sfd || cur_fd == local_sfd)
            {
                if (free_client_index < MAX_CLIENTS)
                {
                    int sfd2 = accept(events[i].data.fd, NULL, NULL);
                    if (sfd2 == -1)
                    {
                        perror("connection error");
                        exit(-1);
                    }
                    clients[free_client_index].sfd = sfd2;

                    /*
                     * TODO: add what happens after accepting
                     *
                     */
                }
            }
            else
            {
                handle_client(cur_fd, &clients);
            }
        }
    }

    close(epoll_fd);
    close(inet_sfd);
    close(local_sfd);

    return 0;
}

void process_input(int argc, char **argv, int *port_number, char *socket_path)
{
    if (argc != 3)
    {
        perror("Usage: ./server <port> <socket path>");
        exit(-1);
    }

    sscanf(argv[1], "%d", port_number);
    sscanf(argv[2], "%s", socket_path);
}
void handle_client(int sfd, struct client_s *clients)
{
}