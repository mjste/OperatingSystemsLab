all:
	gcc -Wall -c common.c
	gcc -Wall -o server server.c -pthread common.o
	gcc -Wall -o client client.c -pthread common.o

