all:
	gcc -c -Wall common.c
	gcc -Wall -o pizzeria pizzeria.c common.o -lpthread -lrt
	gcc -Wall -o baker baker.c common.o -lpthread -lrt
	gcc -Wall -o deliverer deliverer.c common.o -lpthread -lrt

clean:
	rm pizzeria baker deliverer common.o || true