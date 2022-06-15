#include "common.h"

#define MAX_EPOLL_EVENTS 2

#define LISTEN_NUM 10
#define MAX_CLIENTS 20

struct client_s
{
    int sfd;
    enum ttmessage c_play_sign;
    char name[NAME_LENGTH];
};

void process_input(int argc, char **argv, int *port_number, char *socket_path);
void handle_active_client(int sfd, int epoll_fd);
void *ping_fun(void *dummy);
int does_client_exist(char *name);
int neighbour(int index);
void add_client(int safd, int epoll_fd);

int free_client_index = 0;
struct client_s clients[MAX_CLIENTS];
pthread_mutex_t free_client_index_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv)
{
    char socket_path[PATH_MAX];
    int port_number;
    process_input(argc, argv, &port_number, socket_path);

    // // socket timeout
    // struct timeval timeout;
    // timeout.tv_sec = 10;
    // timeout.tv_usec = 0;

    int local_sfd, inet_sfd, epoll_fd;
    int running = 1;
    int epoll_count;
    struct sockaddr_in saddr_in;
    struct sockaddr_un saddr_un;
    size_t bytes_read;
    char read_buffer[BUFFER_SIZE];
    struct epoll_event event, events[2];

    // init
    {
        srand(time(NULL));
        // creating sockets
        {
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
        }

        // filling structures
        {
            saddr_in.sin_family = AF_INET;
            saddr_in.sin_port = htons(port_number);
            saddr_in.sin_addr.s_addr = htonl(INADDR_ANY);

            saddr_un.sun_family = AF_UNIX;
            strcpy(saddr_un.sun_path, socket_path);
        }

        // binding sockets
        {
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
        }

        // epoll
        {
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
        }

        // listen
        {

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
        }

        // init clients
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            clients[i].name[0] = 0;
            clients[i].sfd = -1;
        }
    }

    printf("Successfully opened server\n");

    pthread_t ping_thread;
    pthread_create(&ping_thread, NULL, ping_fun, NULL);

    while (running)
    {
        epoll_count = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS, 30000);
        // printf("Hello there\n");
        for (int i = 0; i < epoll_count; i++)
        {
            int cur_fd = events[i].data.fd;
            if (cur_fd == inet_sfd || cur_fd == local_sfd)
            {
                add_client(cur_fd, epoll_fd);
            }
            else
            {
                handle_active_client(cur_fd, epoll_fd);
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
        printf("Usage: ./server <port> <socket path>\n");
        exit(-1);
    }
    sscanf(argv[1], "%d", port_number);
    sscanf(argv[2], "%s", socket_path);
}

void handle_active_client(int sfd, int epoll_fd)
{
}

void *ping_fun(void *dummy)
{
    while (1)
    {
        sleep(20);
        printf("pinging\n");
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            // clients must be initialized first
        }
    }
    return NULL;
}

int does_client_exist(char *name)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (strcmp(name, clients[i].name) == 0)
        {
            return 1;
        }
    }
    return 0;
}

void add_client(int safd, int epoll_fd)
{
    int sfd2 = accept(safd, NULL, NULL);
    if (sfd2 == -1)
    {
        return;
    }
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    if (setsockopt(sfd2, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
    {
        close(sfd2);
        return;
    }
    if (setsockopt(sfd2, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == -1)
    {
        close(sfd2);
        return;
    }
    printf("Adding client\n");

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    if (read(sfd2, buffer, 1) == -1)
    {
        close(sfd2);
        return;
    }
    int size = buffer[0];
    printf("size: %d\n", size);
    read(sfd2, buffer, size);
    printf("name: %s\n", buffer);

    if (does_client_exist(buffer))
    {
        printf("such client exists\n");
        close(sfd2);
        return;
    }

    // put data into clients
    clients[free_client_index].sfd = sfd2;
    strncpy(clients[free_client_index].name, buffer, NAME_LENGTH);

    int neigh = neighbour(free_client_index);
    if (clients[neigh].sfd == -1)
    {
        char msg = WAIT;
        if (write(sfd2, &msg, 1) != 1)
        {
            close(sfd2);
            return;
        }
    }
    else
    {
        char sign = rand() % 2;
        char sign1;
        char sign2;
        if (sign)
        {
            sign1 = CIRCLE;
            sign2 = CROSS;
        }
        else
        {
            sign1 = CROSS;
            sign2 = CIRCLE;
        }
        clients[free_client_index].c_play_sign = sign1;
        clients[neigh].c_play_sign = sign2;

        memset(buffer, 0, BUFFER_SIZE);
        buffer[0] = sign1;
        buffer[1] = strlen(clients[neigh].name);
        strcpy(buffer + 2, clients[neigh].name);
        write(sfd2, buffer, strlen(buffer));

        memset(buffer, 0, BUFFER_SIZE);
        buffer[0] = sign2;
        buffer[1] = strlen(clients[free_client_index].name);
        strcpy(buffer + 2, clients[free_client_index].name);
        write(clients[neigh].sfd, buffer, strlen(buffer));
    }
    printf("successfully ended adding\n");
    free_client_index += 1;
}

void delete_client(int fd, int epoll_fd)
{
}

int neighbour(int index)
{
    return index - 1 + 2 * ((index + 1) % 2);
}