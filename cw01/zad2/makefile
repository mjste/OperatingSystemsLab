CC = gcc -std=c11
FLAGS = -Wall -g

all: static raport

static: make_libs
	$(CC) $(FLAGS) main.c -o main ../zad1/liblibrary.a

shared: make_libs
	$(CC) $(FLAGS) main.c -o main -llibrary -L../zad1 -Wl,-rpath=../zad1

make_libs:
	make -C ../zad1

raport:
	./main `cat ../files/arg1.txt` > raport2.txt
	./main `cat ../files/arg2.txt` >> raport2.txt

clean:
	rm main || true
	rm tmp || true