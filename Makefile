CC = gcc
SRC = src/*.c lib/*.c
OBJ = *.o
CFLAGS = -Ilib -Wall -Wextra -std=c99 -pedantic
OFLAGS = -Llib -lnavvdf

all:
	$(CC) $(SRC) $(CFLAGS) -c
	$(CC) -o nhschema $(OBJ) $(OFLAGS)

