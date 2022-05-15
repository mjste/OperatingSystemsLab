#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main()
{

    // albo przypisać wartość stałą
    // albo stworzyć plik tak jak się nazywa

    char i; // wiadomośc do wysłania
    key_t klucz = ftok("./nazwa_kolejki", 'p');
    // key_t klucz = 1;
    int id_kolejki_kom = msgget(klucz, IPC_CREAT | 0600);
    for (i = 0; i < 20; i++)
        msgsnd(id_kolejki_kom, &i, sizeof(char), 0);
    return 0;
}

// Działa