#include "common.h"
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

void set_sigaction();
void sigint_handler(int);
void init_queue();
void remove_queue();
void connect_to_server();
void disconnect_from_server();
int string_option(char *command);

int msgq_id;
int server_msgq_id;
int client_id;
key_t key;

int main()
{
    init_queue();
    atexit(remove_queue);
    set_sigaction();
    connect_to_server();
    atexit(disconnect_from_server);

    char buffer[MESSAGE_SIZE];
    char buffer1[MESSAGE_SIZE];
    char buffer2[MESSAGE_SIZE];
    char buffer3[MESSAGE_SIZE];
    char buffer4[MESSAGE_SIZE];
    struct packet r_packet;

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
            puts("Reply: ");
            r_packet.type = CS_LIST;
            r_packet.packet_message.sender_id = client_id;
            memset(r_packet.packet_message.message, 0, MESSAGE_SIZE);
            msgsnd(server_msgq_id, &r_packet, sizeof(struct packet_message), 0);
            msgrcv(msgq_id, &r_packet, sizeof(struct packet_message), SC_LIST, 0);
            puts(r_packet.packet_message.message);
            break;
        }
        case CS_TO_ONE:
        {
            r_packet.type = CS_TO_ONE;
            r_packet.packet_message.receiver_id = atoi(buffer2);
            r_packet.packet_message.sender_id = client_id;
            strcpy(r_packet.packet_message.message, buffer3);
            msgsnd(server_msgq_id, &r_packet, sizeof(struct packet_message), 0);
            puts("Message sent");
            break;
        }
        case CS_TO_ALL:
        {
            r_packet.type = CS_TO_ALL;
            r_packet.packet_message.sender_id = client_id;
            strcpy(r_packet.packet_message.message, buffer4);
            msgsnd(server_msgq_id, &r_packet, sizeof(struct packet_message), 0);
            puts("Message sent");
            break;
        }
        case CS_STOP:
        {
            exit(0);
            break;
        }
        case C_READ:
        {
            while (msgrcv(msgq_id, &r_packet, sizeof(struct packet_message), SC_SEND, IPC_NOWAIT) > 0)
            {
                printf("from: %d\n", r_packet.packet_message.sender_id);
                printf("message: %s\n", r_packet.packet_message.message);
            }
            if (msgrcv(msgq_id, &r_packet, sizeof(struct packet_message), SC_STOP, IPC_NOWAIT) > 0)
            {
                puts("Server was shut down. Shutting down");
                exit(0);
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
    char i = '2';
    while (1)
    {
        if (i == 255)
        {
            puts("Sender limit achieved");
            exit(1);
        }
        key = ftok(getenv("HOME"), i);
        msgq_id = msgget(key, IPC_CREAT | IPC_EXCL | 0600);
        if (msgq_id != -1)
        {
            break;
        }
        i++;
    }
}

void remove_queue()
{
    if (msgctl(msgq_id, IPC_RMID, NULL) == -1)
    {
        perror("remove_queue");
        exit(errno);
    }
    puts("Queue removed successfully");
}

void connect_to_server()
{
    key_t server_key = ftok(getenv("HOME"), SERVER_KEY_NUMBER);
    if (server_key == -1)
    {
        perror("connect_to_server");
        exit(errno);
    }
    server_msgq_id = msgget(server_key, 0);
    if (server_msgq_id == -1)
    {
        perror("connect_to_server");
        exit(errno);
    }
    struct packet new_packet;
    new_packet.type = CS_INIT;
    new_packet.packet_message.sender_id = 0;
    new_packet.packet_message.queue_key = key;
    memset(new_packet.packet_message.message, 0, MESSAGE_SIZE);
    msgsnd(server_msgq_id, &new_packet, sizeof(struct packet_message), 0);
    msgrcv(msgq_id, &new_packet, sizeof(struct packet_message), 0, 0);
    client_id = new_packet.packet_message.receiver_id;
    printf("client_id: %d\n", client_id);
}

void disconnect_from_server()
{
    struct packet new_packet;
    new_packet.type = CS_STOP;
    new_packet.packet_message.sender_id = client_id;
    new_packet.packet_message.receiver_id = 0;
    memset(new_packet.packet_message.message, 0, MESSAGE_SIZE);
    msgsnd(server_msgq_id, &new_packet, sizeof(struct packet_message), 0);
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