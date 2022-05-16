#define CLIENTS_NUMBER 256
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

mqd_t queue;
char message_buffer[PACKET_SIZE];

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
    memset(&clients, 0, sizeof(clients)); //
    open_log_file();                      //
    init_receive_queue();
    set_sigaction();
    atexit(exit0);
    atexit(stop_clients);

    int bytes;
    while (1)
    {
        while (1)
        {
            bytes = mq_receive(queue, message_buffer, PACKET_SIZE, 0);
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
    int result = mq_close(queue);
    result = mq_unlink("/0");
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
    struct mq_attr mqattr;
    mqattr.mq_msgsize = PACKET_SIZE;
    mqattr.mq_flags = 0;
    mqattr.mq_curmsgs = 0;
    mqattr.mq_maxmsg = 10;
    if ((queue = mq_open("/0", O_RDONLY | O_CREAT, 0644, &mqattr)) == -1)
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
    char sender_id = message_buffer[1];
    char buffer_to_send[PACKET_SIZE];
    memset(buffer_to_send, 0, PACKET_SIZE);

    switch (message_buffer[0])
    {
    case CS_LIST:
    {
        buffer_to_send[MF_TYPE] = SC_LIST;
        buffer_to_send[MF_FROM] = -1;
        buffer_to_send[MF_TO] = sender_id;
        memset(buffer_to_send + MF_MESSAGE, 0, MESSAGE_SIZE);

        char tmp_buffer[20];
        for (int i = 0; i < CLIENTS_NUMBER; i++)
        {
            if (clients[i] != 0)
            {
                sprintf(tmp_buffer, "%d,", i);
                strcat(buffer_to_send + MF_MESSAGE, tmp_buffer);
            }
        }
        if (mq_send(clients[(int)sender_id], buffer_to_send, sizeof(buffer_to_send), buffer_to_send[0]) == -1)
        {
            fputs("failed to send", stderr);
        }

        break;
    }
    case CS_STOP:
    {
        int sender_id = message_buffer[MF_FROM];
        mq_close(clients[sender_id]);
        clients[sender_id] = 0;
        break;
    }
    case CS_TO_ALL:
    {
        buffer_to_send[0] = SC_SEND;
        strcpy(buffer_to_send + MF_MESSAGE, message_buffer + MF_MESSAGE);
        buffer_to_send[MF_FROM] = sender_id;
        for (int i = 0; i < CLIENTS_NUMBER; i++)
        {
            unsigned char index = i;
            if (clients[index] != 0)
            {
                buffer_to_send[MF_TO] = index;
                mq_send(clients[index], buffer_to_send, PACKET_SIZE, 0);
            }
        }
        break;
    }
    case CS_TO_ONE:
    {
        message_buffer[MF_TYPE] = SC_SEND;
        mq_send(clients[(int)message_buffer[MF_TO]], message_buffer, PACKET_SIZE, 0);
        break;
    }
    case CS_INIT:
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
            clients[i] = mq_open(message_buffer + MF_MESSAGE, O_WRONLY);
            buffer_to_send[MF_TO] = (unsigned char)i;
            buffer_to_send[MF_FROM] = 0;
            buffer_to_send[MF_TYPE] = SC_INIT;
            mq_send(clients[i], buffer_to_send, PACKET_SIZE, 0);
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
    char message_type[20];

    switch (message_buffer[MF_TYPE])
    {
    case CS_INIT:
        strcpy(message_type, "CS_INIT");
        break;
    case CS_LIST:
        strcpy(message_type, "CS_LIST");
        break;
    case CS_STOP:
        strcpy(message_type, "CS_STOP");
        break;
    case CS_TO_ALL:
        strcpy(message_type, "CS_TO_ALL");
        break;
    case CS_TO_ONE:
        strcpy(message_type, "CS_TO_ONE");
        break;
    default:
        strcpy(message_type, "UNKNOWN");
        break;
    }

    char log_buffer[200];
    sprintf(log_buffer, "%s, type_num=%d, %s, %d, %s\n", s, (int)message_buffer[MF_TYPE], message_type, (int)message_buffer[MF_FROM], message_buffer + MF_MESSAGE);
    printf("%s", log_buffer);
    fprintf(fp, "%s", log_buffer);
}

void stop_clients()
{
    char buffer_to_send[PACKET_SIZE];
    buffer_to_send[MF_TYPE] = SC_STOP;
    for (int i = 1; i < CLIENTS_NUMBER; i++)
    {
        if (clients[i] != 0)
        {
            mq_send(clients[i], buffer_to_send, PACKET_SIZE, 0);
        }
    }
}