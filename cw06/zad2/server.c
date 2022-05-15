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
void stop_clients();

int main()
{
    memset(&clients, 0, sizeof(clients));
    open_log_file();
    init_receive_queue();
    set_sigaction();
    atexit(exit0);
    atexit(stop_clients);

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
    // remove message queue
    int result = msgctl(msgq_id, IPC_RMID, NULL);
    if (result == -1)
    {
        perror("exit0");
        exit(errno);
    }
    puts("Queue removed successfully");

    fclose(fp);
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
    exit(0);
}

void handle_packet()
{
    save_log();
    int sender_id = received_packet.packet_message.sender_id;
    struct packet new_packet;

    switch (received_packet.type)
    {
    case CS_LIST:
    {
        new_packet.type = SC_LIST;
        new_packet.packet_message.sender_id = 0;
        new_packet.packet_message.receiver_id = sender_id;
        char *message = new_packet.packet_message.message;
        memset(message, 0, sizeof(struct packet_message));
        char buffer[MESSAGE_SIZE];
        for (int i = 0; i < CLIENTS_NUMBER; i++)
        {
            if (clients[i] != 0)
            {
                sprintf(buffer, "%d,", i);
                strcat(message, buffer);
            }
        }
        msgsnd(clients[sender_id], &new_packet, sizeof(struct packet_message), 0);
        break;
    }
    case CS_STOP:
    {
        msgctl(clients[sender_id], IPC_RMID, NULL);
        clients[sender_id] = 0;
        break;
    }
    case CS_TO_ALL:
    {
        char *message = received_packet.packet_message.message;
        new_packet.type = SC_SEND;
        strcpy(new_packet.packet_message.message, message);
        new_packet.packet_message.sender_id = sender_id;
        for (int i = 0; i < CLIENTS_NUMBER; i++)
        {
            if (clients[i] != 0)
            {
                new_packet.packet_message.receiver_id = i;
                msgsnd(clients[i], &new_packet, sizeof(struct packet_message), 0);
            }
        }
        break;
    }

    case CS_TO_ONE:
    {
        received_packet.type = SC_SEND;
        int receiver = received_packet.packet_message.receiver_id;
        msgsnd(clients[receiver], &received_packet, sizeof(struct packet_message), 0);
        break;
    }
    case CS_INIT:
        // find first empty space in clients
        {
            int i;
            for (i = 1; i < CLIENTS_NUMBER; i++)
            {
                if (clients[i] == 0)
                {
                    break;
                }
            }
            if (i < CLIENTS_NUMBER)
            {
                clients[i] = msgget(received_packet.packet_message.queue_key, 0);
                new_packet.type = SC_INIT;
                new_packet.packet_message.sender_id = 0;
                new_packet.packet_message.receiver_id = i;
                msgsnd(clients[i], &new_packet, sizeof(struct packet_message), 0);
            }

            break;
        }
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
    memset(s, 0, sizeof(s));
    time_t t = time(NULL);
    struct tm *tmp = localtime(&t);
    strftime(s, sizeof(s), "%c", tmp);

    fprintf(fp, "%s %ld %d %s\n", s, received_packet.type, received_packet.packet_message.sender_id, received_packet.packet_message.message);
    fprintf(stdout, "%s %ld %d %s\n", s, received_packet.type, received_packet.packet_message.sender_id, received_packet.packet_message.message);
}

void stop_clients()
{
    struct packet end_packet;
    end_packet.type = SC_STOP;
    for (int i = 1; i < CLIENTS_NUMBER; i++)
    {
        if (clients[i] != 0)
        {
            msgsnd(clients[i], &end_packet, sizeof(struct packet_message), 0);
        }
    }
}