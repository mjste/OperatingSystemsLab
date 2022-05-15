#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
    char w;
    key_t klucz = ftok("./nazwa_kolejki", 'p');
    // key_t klucz = 1;
    int id_kolejki_kom = msgget(klucz, 0600);

    while (1)
    {
        // pobiera dane z kolejki
        if (msgrcv(id_kolejki_kom, &w, sizeof(char), 0, IPC_NOWAIT) < 0)
            break;
        printf("otrzymano %d\n", (int)w);
    }

    msgctl(id_kolejki_kom, IPC_RMID, NULL); // usuwa kolejkę
    return 0;
}

// Działa