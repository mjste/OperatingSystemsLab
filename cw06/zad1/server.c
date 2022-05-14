#define CLIENTS_NUMBER 256

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

int msgq_id;
struct packet received_packet;
int clients[CLIENTS_NUMBER];
FILE *fp;

void exit0();
void init_receive_queue();
void set_sigaction();
void sigint_handler(int signum);
void handle_packet();
void open_log_file();
void save_log();

int main()
{
    memset(&clients, 0, sizeof(clients));
    open_log_file();
    init_receive_queue();
    set_sigaction();
    atexit(exit0);

    int bytes;
    while (1)
    {
        while (1)
        {
            bytes = msgrcv(msgq_id, &received_packet, sizeof(struct packet_message), CS_STOP, IPC_NOWAIT);
            if (bytes == -1)
                break;
            else
                handle_packet();
        }
        while (1)
        {
            bytes = msgrcv(msgq_id, &received_packet, sizeof(struct packet_message), CS_LIST, IPC_NOWAIT);
            if (bytes == -1)
                break;
            else
                handle_packet();
        }
        while (1)
        {
            bytes = msgrcv(msgq_id, &received_packet, sizeof(struct packet_message), ANY_REQUEST, IPC_NOWAIT);
            if (bytes == -1)
                break;
            else
                handle_packet();
        }
        sleep(1);
    }
    return 0;
}

void exit0()
{
    if (msgctl(msgq_id, IPC_RMID, NULL) == -1)
    {
        perror(strerror(errno));
        exit(errno);
    }
}

void init_receive_queue()
{
    key_t rec_qkey;
    rec_qkey = ftok(getenv("HOME"), SERVER_KEY_NUMBER);

    if (rec_qkey == -1)
    {
        perror(strerror(errno));
        exit(errno);
    }
    puts("Key created succesfully");

    msgq_id = msgget(rec_qkey, IPC_CREAT | 0600);
    if (msgq_id == -1)
    {
        perror(strerror(errno));
        exit(errno);
    }
    puts("Queue opened succesfully");
}

void set_sigaction()
{
    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);
}

void sigint_handler(int signum)
{
    exit0();
    exit(0);
}

void handle_packet()
{
    save_log();
    int sender_id = received_packet.packet_message.sender_id;

    switch (received_packet.type)
    {
    case CS_LIST:
    {
        struct packet new_packet;
        new_packet.type = SC_SEND;
        new_packet.packet_message.sender_id = 0;
        new_packet.packet_message.receiver_id = sender_id;
        char *message = new_packet.packet_message.message;
        memset(message, 0, sizeof(new_packet.packet_message.message));
        char buffer[MESSAGE_SIZE];
        for (int i = 0; i < CLIENTS_NUMBER; i++)
        {
            if (clients[i] != 0)
            {
                sprintf(buffer, "%d,", clients[i]);
                strcat(message, buffer);
            }
        }

        msgsnd(sender_id, &new_packet, sizeof(struct packet_message), 0);

        break;
    }
    case CS_STOP:
        break;
    case CS_TO_ALL:
        break;
    case CS_TO_ONE:
        break;
    case CS_INIT:
        break;
    }
}

void open_log_file()
{
    if ((fp = fopen("server.log", "a")) == NULL)
    {
        perror(strerror(errno));
        exit(errno);
    }
}

void close_log_file()
{
    if (fclose(fp) == EOF)
    {
        perror(strerror(errno));
        exit(errno);
    }
}

void save_log()
{
    char s[64];
    fprintf(fp, "%s %ld %d %s\n", s, received_packet.type, received_packet.packet_message.sender_id, received_packet.packet_message.message);
    fprintf(stdout, "%s %ld %d %s\n", s, received_packet.type, received_packet.packet_message.sender_id, received_packet.packet_message.message);
}