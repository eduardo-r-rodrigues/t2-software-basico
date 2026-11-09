CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2

all: main test

main: main.c mymemory.c mymemory.h
	$(CC) $(CFLAGS) -o $@ main.c mymemory.c

test: test.c mymemory.c mymemory.h
	$(CC) $(CFLAGS) -o $@ test.c mymemory.c

run: all
	./main
	./test

check: main.c test.c mymemory.c mymemory.h
	$(CC) $(CFLAGS) -g -fsanitize=address,undefined -o main_asan main.c mymemory.c
	$(CC) $(CFLAGS) -g -fsanitize=address,undefined -o test_asan test.c mymemory.c
	./main_asan > /dev/null
	./test_asan > /dev/null
	@echo "Sem erros de memoria nem vazamentos."

clean:
	rm -f main test main_asan test_asan

.PHONY: all run check clean
