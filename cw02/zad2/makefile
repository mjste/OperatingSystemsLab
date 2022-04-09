input_file=testfile
output_file=raport.txt 


all: sys lib

sys:
	gcc -Wall main_sys.c -o main_sys

lib:
	gcc -Wall main_lib.c -o main_lib

clean:
	rm main_lib || true
	rm main_sys || true

create_testfile:
	dd if=/dev/urandom of=testfile count=1000

test: create_testfile sys lib
	./test.sh
