CC ?= gcc
CFLAGS += -Wno-format -Wall -Wextra -pedantic

test: test.c
	$(CC) $(CFLAGS) test.c
