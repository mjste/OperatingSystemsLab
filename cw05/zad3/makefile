all:
	gcc -Wall consumer.c -o consumer
	gcc -Wall producer.c -o producer

clean:
	rm producer consumer out.txt || true

produce:
	./producer fifo 1 1.txt 10
	./producer fifo 2 2.txt 10
	./producer fifo 3 3.txt 10