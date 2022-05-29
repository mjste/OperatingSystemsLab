all:
	gcc -Wall -c utils.c
	gcc -Wall -o prog prog.c utils.o -pthread

clean:
	rm prog || true
