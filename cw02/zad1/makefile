all: sys lib

sys:
	gcc -Wall main_sys.c -o main_sys -Wno-unused-variable

lib:
	gcc -Wall main_lib.c -o main_lib -Wno-unused-variable

test: create_testfile sys lib
	./test.sh

clean:
	rm main_lib || true
	rm main_sys || true
	rm testfile || true
	rm tmp || true

create_testfile:
	dd if=/dev/urandom of=testfile count=1000