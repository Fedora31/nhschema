CC = gcc
SRC = src/*.c
CFLAGS = -Ilib -Wall -Wextra -std=c89 -pedantic

all:
	$(CC) -o nhcdbu $(SRC) $(CFLAGS)

