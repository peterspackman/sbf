CFLAGS=-Wall -Wextra -std=c11 -I include
CC=gcc
.PHONY: clean test tidy

all: test

test_write: include/sbf.h tests/test_write.c bin
	$(CC) $(CFLAGS) -o bin/test_write tests/test_write.c

test_read: include/sbf.h tests/test_read.c bin
	$(CC) $(CFLAGS) -o bin/test_read tests/test_read.c

bin:
	mkdir -p bin

test: test_write test_read
	./bin/test_write
	./bin/test_read

tidy:
	./scripts/cleanup-format -i test.c sbf.h

clean:
	rm -rf test.sbf bin/test_write bin/test_read
