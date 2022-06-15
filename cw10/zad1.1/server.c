#include "common.h"

#define LISTEN_NUM 10
#define EPOLL_EVENT_NUM 10
#define MAX_CLIENTS 20

struct client_s
{
    int fd;
    int symbol;
    char name[NAME_LENGTH];
};

void process_input(int argc, char **argv, int *port_numberp);
void init_sockets(int *local_fd, int *inet_fd, int *epoll_fd, char *path, int port_number);
int does_client_exists(char *name);
void add_client(int fd, int epoll_fd);
void handle_client(int fd, int epoll_fd);
void init_clients();
int find_index();
void make_pair(int index);

struct client_s clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv)
{
    int local_sfd, inet_sfd, epoll_fd, port_number;
    process_input(argc, argv, &port_number);
    init_sockets(&local_sfd, &inet_sfd, &epoll_fd, argv[2], port_number);
    init_clients();
    srand(time(NULL));

    int running = 1;
    int event_count;
    struct epoll_event events[EPOLL_EVENT_NUM];
    while (running)
    {
        event_count = epoll_wait(epoll_fd, events, EPOLL_EVENT_NUM, 30000);
        for (int i = 0; i < event_count; i++)
        {
            int fd = events[i].data.fd;
            if (fd == local_sfd || fd == inet_sfd)
            {
                add_client(fd, epoll_fd);
            }
            else
            {
                handle_client(fd, epoll_fd);
            }
        }
    }
    return 0;
}

void process_input(int argc, char **argv, int *port_numberp)
{
    if (argc != 3)
        fatal("Incorrect argument number");
    sscanf(argv[1], "%d", port_numberp);
}

void init_clients()
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].fd = -1;
        clients[i].name[0] = 0;
    }
}

void init_sockets(int *local_fd, int *inet_fd, int *epoll_fd, char *path, int port_number)
{
    // create
    *local_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (*local_fd == -1)
        fatal("local_sfd");
    *inet_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*inet_fd == -1)
        fatal("inet_sfd");

    // remove
    if (remove(path) == -1 && errno != ENOENT)
        fatal("rm local socket");

    // bind
    struct sockaddr_un sun;
    sun.sun_family = AF_UNIX;
    strcpy(sun.sun_path, path);
    if (bind(*local_fd, (struct sockaddr *)&sun, (socklen_t)sizeof(sun)) == -1)
        fatal("bind local");

    struct sockaddr_in sin;
    sin.sin_port = htons(port_number);
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    sin.sin_family = AF_INET;
    if (bind(*inet_fd, (struct sockaddr *)&sin, (socklen_t)sizeof(sin)) == -1)
        fatal("bind inet");

    // listen
    if (listen(*local_fd, LISTEN_NUM) == -1)
        fatal("listen local");
    if (listen(*inet_fd, LISTEN_NUM) == -1)
        fatal("listen inet");

    // epoll
    *epoll_fd = epoll_create1(0);
    if (*epoll_fd == -1)
        fatal("epoll create");

    struct epoll_event event;
    event.data.fd = *local_fd;
    event.events = EPOLLIN;
    if (epoll_ctl(*epoll_fd, EPOLL_CTL_ADD, *local_fd, &event) == -1)
        fatal("epoll_ctl local");
    event.data.fd = *inet_fd;
    if (epoll_ctl(*epoll_fd, EPOLL_CTL_ADD, *inet_fd, &event) == -1)
        fatal("epoll_ctl inet");
    return;
}

void add_client(int fd, int epoll_fd)
{
    printf("adding client\n");
    int sfd2 = accept(fd, NULL, NULL);
    if (sfd2 == -1)
        fatal("can't accept");

    // set timeout
    struct timeval timeout1, timeout2;
    timeout1.tv_sec = 10;
    timeout1.tv_usec = 0;
    timeout2.tv_sec = 60;
    timeout2.tv_usec = 0;

    if (setsockopt(sfd2, SOL_SOCKET, SO_RCVTIMEO, &timeout1, sizeof(timeout1)) == -1 ||
        setsockopt(sfd2, SOL_SOCKET, SO_SNDTIMEO, &timeout2, sizeof(timeout2)) == -1)
    {
        printf("adding client - fail\n");
        close(sfd2);
        return;
    }

    // get name
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    if (read(sfd2, buffer, 1) <= 0)
    {
        printf("adding client - fail\n");
        close(sfd2);
        return;
    }
    int namesize = buffer[0];
    if (namesize <= 0 || read(sfd2, buffer, namesize) != namesize)
    {
        printf("adding client - fail\n");
        close(sfd2);
        return;
    }
    // name in buffer
    // check if exists another client with identical name
    if (does_client_exists(buffer))
    {
        printf("Client has incorrect name\n");
        buffer[0] = WRONG_MESSAGE;
        sprintf(buffer + 2, "This name already exists\n");
        buffer[1] = strlen(buffer + 2);
        write(sfd2, buffer, strlen(buffer));
        close(sfd2);
        return;
    }

    pthread_mutex_lock(&clients_mutex);
    int index = find_index();
    if (index == -1)
    {
        printf("adding client - fail - too many clients\n");
        memset(buffer, 0, BUFFER_SIZE);
        buffer[0] = WRONG_MESSAGE;
        sprintf(buffer + 2, "Client limit reached\n");
        buffer[1] = strlen(buffer + 2);
        write(sfd2, buffer, strlen(buffer));
        close(sfd2);
        pthread_mutex_unlock(&clients_mutex);
        return;
    }
    // clients name is good and there's place for it
    printf("Name: %s\n", buffer);
    clients[index].fd = sfd2;
    strcpy(clients[index].name, buffer);
    pthread_mutex_unlock(&clients_mutex);

    // add fd to epoll
    struct epoll_event event;
    event.data.fd = sfd2;
    event.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sfd2, &event);

    // send wait to client
    buffer[0] = WAIT;
    write(sfd2, buffer, 1);

    // run make_pair
    if (index % 2 == 1)
        make_pair(index);

    printf("Successfully added client\n");
}

int does_client_exists(char *name)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (strcmp(clients[i].name, name) == 0)
        {
            pthread_mutex_unlock(&clients_mutex);
            return 1;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return 0;
}

int find_index()
{
    // return odd place near neighbour
    for (int i = 0; i < MAX_CLIENTS - 1; i++)
    {
        if (clients[i].fd != -1 && clients[i].fd == -1 && i % 2 == 0)
        {
            return i + 1;
        }
    }
    // return first empty place
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].fd == -1)
        {
            return i;
        }
    }
    return -1;
}

int neighbour_index(int index)
{
    return index - 1 + (index + 1) % 2;
}

void make_pair(int index)
{
    int n_index = neighbour_index(index);
    char symbol1;
    char symbol2;
    if (rand() % 2 == 0)
    {
        symbol1 = CIRCLE;
        symbol2 = CROSS;
    }
    else
    {
        symbol1 = CROSS;
        symbol2 = CIRCLE;
    }
    pthread_mutex_lock(&clients_mutex);
    clients[index].symbol = symbol1;
    clients[n_index].symbol = symbol2;

    // send to index
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = symbol1;
    buffer[1] = FIRST;
    strcpy(buffer + 3, clients[n_index].name);
    buffer[2] = strlen(buffer + 3);
    write(clients[index].fd, buffer, strlen(buffer));

    // send to neighbour
    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = symbol2;
    buffer[1] = SECOND;
    strcpy(buffer + 3, clients[index].name);
    buffer[2] = strlen(buffer + 3);
    write(clients[n_index].fd, buffer, strlen(buffer));
    pthread_mutex_unlock(&clients_mutex);
    return;
}

void delete_client(int fd, int epoll_fd)
{
}

void handle_client(int fd, int epoll_fd)
{
    char msg;
    if (read(fd, &msg, 1) != 1)
    {
        delete_client(fd, epoll_fd);
        return;
    }

    switch (msg)
    {
    case QUIT:
        delete_client(fd, epoll_fd);
        break;

    default:
        break;
    }
}