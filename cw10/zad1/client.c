#include "common.h"

enum connection_type_enum
{
    INET,
    UNIX
};

void process_arguments(int argc, char **argv, char *name, enum connection_type_enum *connection_type, char *address, int *port_number);
void argument_error();

int main(int argc, char **argv)
{
    int sign;
    char name[NAME_LENGTH];
    char opponent_name[NAME_LENGTH];
    enum connection_type_enum connection_type;
    char address[PATH_MAX];
    int port_number;
    process_arguments(argc, argv, name, &connection_type, address, &port_number);

    int sfd;
    switch (connection_type)
    {
    case INET:
    {
        sfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sfd == -1)
        {
            perror("socket inet");
            exit(0);
        }

        struct sockaddr_in saddr_in;
        saddr_in.sin_addr.s_addr = inet_addr(address);
        saddr_in.sin_family = AF_INET;
        saddr_in.sin_port = htons(port_number);

        if (connect(sfd, (struct sockaddr *)&saddr_in, (socklen_t)sizeof(saddr_in)) == -1)
        {
            perror("connect inet");
            exit(0);
        }
        break;
    }
    case UNIX:
    {
        sfd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sfd == -1)
        {
            perror("socket unix");
            exit(0);
        }

        struct sockaddr_un saddr_un;
        saddr_un.sun_family = AF_UNIX;
        strncpy(saddr_un.sun_path, address, 108);

        if (connect(sfd, (struct sockaddr *)&saddr_un, (socklen_t)sizeof(saddr_un)) == -1)
        {
            perror("connect unix");
            exit(0);
        }
        break;
    }
    }

    char buffer[BUFFER_SIZE];
    int read_bytes;
    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = strlen(name);
    strcpy(buffer + 1, name);
    write(sfd, buffer, strlen(buffer));

    memset(buffer, 0, BUFFER_SIZE);
    read(sfd, buffer, 1);

    switch (buffer[0])
    {
    case WAIT:
        read(sfd, buffer, 1);
        break;

    case CROSS:
    case CIRCLE:
    }

    sign = buffer[0];
    strncpy(opponent_name, buffer + 2, buffer[1]);
    printf("You will be matching with %s\n", opponent_name);

    while (1)
    {
        sleep(1);
    }

    return 0;
}

void process_arguments(int argc, char **argv, char *name, enum connection_type_enum *connection_type, char *address, int *port_number)
{
    /*
    name
    type: local | inet
    address: path | ip
    port: - | port

    */
    if (argc == 4)
    {
        sscanf(argv[1], "%s", name);

        if (strcmp(argv[2], "local") == 0)
        {
            *connection_type = UNIX;
            strncpy(name, argv[2], NAME_LENGTH);
        }
        else
        {
            argument_error();
        }
        strncpy(address, argv[3], PATH_MAX);
    }
    else if (argc == 5)
    {
        sscanf(argv[1], "%s", name);

        if (strcmp(argv[2], "inet") == 0)
        {
            *connection_type = INET;
            strncpy(name, argv[1], NAME_LENGTH);
        }
        else
        {
            argument_error();
        }
        strncpy(address, argv[3], PATH_MAX);
        sscanf(argv[4], "%d", port_number);
    }
    else
    {
        argument_error();
    }
}

void argument_error()
{
    printf("Usage: ./client <name> <local|inet> < <path> | <ip> <port> >\n");
    exit(-1);
}
