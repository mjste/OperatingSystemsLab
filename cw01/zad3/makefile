CC = gcc -std=c11
FLAGS = -Wall -g

all: test_a test_b

static: make_libs
	$(CC) $(FLAGS) $(OPTIMIZATION) ../zad2/main.c -o main ../zad1/liblibrary.a

shared: make_libs
	$(CC) $(FLAGS) $(OPTIMIZATION) ../zad2/main.c -o main -llibrary -L../zad1 -Wl,-rpath=../zad1

dynamic: make_libs
	$(CC) $(FLAGS) $(OPTIMIZATION) main.c -D DYNAMIC -ldl -o main -Wl,-rpath=../zad1

make_libs:
	make -C ../zad1

raport_a:
	./main `cat ../files/arg1.txt` >> raport3a.txt
	./main `cat ../files/arg2.txt` >> raport3a.txt

raport_b:
	./main `cat ../files/arg1.txt` >> raport3b.txt
	./main `cat ../files/arg2.txt` >> raport3b.txt

test_a: make_libs
	make static
	echo "Raport 3a" > raport3a.txt
	echo "Static" >> raport3a.txt
	make raport_a
	make shared
	echo "Shared" >> raport3a.txt
	make raport_a
	make dynamic
	echo "Dynamic" >> raport3a.txt
	make raport_a

test_b:
	echo "raport 3b" > raport3b.txt
	for lib in static shared dynamic ; do \
		for o in O0 O1 O2 O3 Os; do \
			make $$lib OPTIMIZATION=-$$o ; \
			echo "$$lib $$o: " >> raport3b.txt ; \
			make raport ; \
		done ; \
	done

clean:
	rm main || true
	rm tmp || true
	

