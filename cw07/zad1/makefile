all:
	gcc -c common.c
	gcc -Wall -o baker baker.c common.o
	gcc -Wall -o deliverer deliverer.c common.o
	gcc -Wall -o pizzeria pizzeria.c common.o

clean:
	rm baker deliverer pizzeria || true