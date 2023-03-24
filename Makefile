CC = gcc
SRC = src/*.c lib/*.c
CFLAGS = -Ilib -Wall -Wextra -std=c89 -pedantic

all:
	$(CC) -o nhschema $(SRC) $(CFLAGS)

