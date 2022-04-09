CC = gcc -std=c11
FLAGS = -Wall -Wno-unused-variable

all: static shared

static:
	$(CC) $(FLAGS) -c library.c
	ar rcs liblibrary.a library.o

shared:
	$(CC) $(FLAGS) -fPIC -c library.c
	$(CC) -shared library.o -o liblibrary.so

clean:
	rm *.o || true
	rm *.so || true
	rm *.a || true