#include "common.h"
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

void set_sigaction();
void sigint_handler(int);
void init_queue();
void remove_queue();
void connect_to_server();
void disconnect_with_server();
int string_option(char *command);

mqd_t queue;
mqd_t server_queue;

int client_id;
char queue_name[30];

int main()
{
    init_queue();
    atexit(remove_queue);
    set_sigaction();
    connect_to_server();
    atexit(disconnect_with_server);

    char buffer[MESSAGE_SIZE];
    char buffer1[MESSAGE_SIZE];
    char buffer2[MESSAGE_SIZE];
    char buffer3[MESSAGE_SIZE];
    char buffer4[MESSAGE_SIZE];

    char message_buffer[PACKET_SIZE];

    while (1)
    {
        fgets(buffer, MESSAGE_SIZE, stdin);
        memset(buffer1, 0, MESSAGE_SIZE);
        memset(buffer2, 0, MESSAGE_SIZE);
        memset(buffer3, 0, MESSAGE_SIZE);
        sscanf(buffer, "%s %s %2000[^\n]", buffer1, buffer2, buffer3);
        strcpy(buffer4, buffer2);
        strcat(buffer4, " ");
        strcat(buffer4, buffer3);

        switch (string_option(buffer1))
        {
        case CS_LIST:
        {
            printf("Reply: ");
            message_buffer[MF_TYPE] = CS_LIST;
            message_buffer[MF_FROM] = (char)client_id;
            memset(message_buffer + MF_MESSAGE, 0, MESSAGE_SIZE);
            mq_send(server_queue, message_buffer, PACKET_SIZE, 0);
            sleep(1);
            mq_receive(queue, message_buffer, PACKET_SIZE, 0);
            puts(message_buffer + MF_MESSAGE);
            break;
        }
        case CS_TO_ONE:
        {
            message_buffer[MF_TYPE] = CS_TO_ONE;
            message_buffer[MF_FROM] = (char)client_id;
            message_buffer[MF_TO] = (char)atoi(buffer2);
            strcpy(message_buffer + MF_MESSAGE, buffer3);
            if (mq_send(server_queue, message_buffer, PACKET_SIZE, 0) != -1)
                puts("Message sent successfully");
            else
                puts("Message couldn't be send");

            break;
        }
        case CS_TO_ALL:
        {
            message_buffer[MF_TYPE] = CS_TO_ALL;
            message_buffer[MF_FROM] = (char)client_id;
            strcpy(message_buffer + MF_MESSAGE, buffer4);
            if (mq_send(server_queue, message_buffer, PACKET_SIZE, 0) == -1)
                puts("Sending error");
            else
                puts("Message sent successfully");
            break;
        }
        case CS_STOP:
        {
            exit(0);
            break;
        }
        case C_READ:
        {
            while (mq_receive(queue, message_buffer, PACKET_SIZE, 0) > 0)
            {
                int type = message_buffer[MF_TYPE];
                if (type == SC_SEND || type == SC_LIST)
                {
                    printf("from: %d\n", (int)message_buffer[MF_FROM]);
                    printf("message: %s\n", message_buffer + MF_MESSAGE);
                }
                else
                {
                    puts("Server was shut down");
                    fflush(stdout);
                    exit(0);
                }
            }
            break;
        }
        case UNKNOWN_REQUEST:
        {
            puts("Unknown request");
            break;
        }
        }
    }

    return 0;
}

void set_sigaction()
{
    struct sigaction siga;
    siga.sa_flags = 0;
    sigemptyset(&siga.sa_mask);
    siga.sa_handler = sigint_handler;
    sigaction(SIGINT, &siga, NULL);
}

void sigint_handler(int signum)
{
    exit(0);
}

void init_queue()
{
    unsigned char i = 1;
    struct mq_attr mqattr;
    mqattr.mq_flags = 0;
    mqattr.mq_msgsize = PACKET_SIZE;
    mqattr.mq_curmsgs = 0;
    mqattr.mq_maxmsg = 10;
    while (1)
    {
        if (i == 255)
        {
            puts("Sender limit achieved");
            exit(1);
        }
        queue_name[0] = '/';
        sprintf(queue_name + 1, "%d", i);
        if ((queue = mq_open(queue_name, O_RDONLY | O_CREAT | O_EXCL | O_NONBLOCK, 0600, &mqattr)) != -1)
        {
            break;
        }
        i++;
    }
    puts(queue_name);
}

void remove_queue()
{
    if (mq_close(queue) == -1)
    {
        perror("close queue");
        exit(errno);
    }
    if (mq_unlink(queue_name) == -1)
    {
        perror("remove queue");
        exit(errno);
    }
    puts("Queue removed successfully");
}

void connect_to_server()
{
    if ((server_queue = mq_open("/0", O_WRONLY)) == -1)
    {
        perror("connection to server");
        exit(errno);
    }
    char message_buffer[PACKET_SIZE];
    message_buffer[MF_TYPE] = CS_INIT;
    message_buffer[MF_FROM] = 0;
    message_buffer[MF_TO] = 0;
    strcpy(message_buffer + MF_MESSAGE, queue_name);
    mq_send(server_queue, message_buffer, PACKET_SIZE, 0);
    sleep(1);
    mq_receive(queue, message_buffer, PACKET_SIZE, 0);
    client_id = (int)message_buffer[MF_TO];
    printf("client_id: %d\n", client_id);
}

void disconnect_with_server()
{
    char message_buffer[PACKET_SIZE];
    message_buffer[MF_TYPE] = CS_STOP;
    message_buffer[MF_FROM] = (char)client_id;
    message_buffer[MF_TO] = 0;
    memset(message_buffer + MF_MESSAGE, 0, MESSAGE_SIZE);
    if (mq_send(server_queue, message_buffer, PACKET_SIZE, 0) != -1)
    {
        puts("Disconnected succesfully");
    }
}

int string_option(char *command)
{
    if (strcmp("list", command) == 0)
    {
        return CS_LIST;
    }
    else if (strcmp("2all", command) == 0)
    {
        return CS_TO_ALL;
    }
    else if (strcmp("2one", command) == 0)
    {
        return CS_TO_ONE;
    }
    else if (strcmp("stop", command) == 0)
    {
        return CS_STOP;
    }
    else if (strcmp("read", command) == 0)
    {
        return C_READ;
    }
    else
    {
        return UNKNOWN_REQUEST;
    }
}