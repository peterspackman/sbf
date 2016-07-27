.PHONY: clean test tidy

all: sbf_test

sbf_test: include/sbf.h tests/test.c bin
	gcc -std=c11 -I include -Wall -Wextra -o bin/test tests/test.c

bin:
	mkdir -p bin

test: sbf_test
	time ./bin/test

tidy:
	./scripts/cleanup-format -i test.c sbf.h

clean:
	rm -rf test.sbf bin/test
