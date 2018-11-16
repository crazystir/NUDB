CC=gcc
HEADERS=$(wildcard include/*.h)
SOURCES=$(wildcard src/*.c)
OBJ=$(SOURCES:.c=.o)

CFLAG=-g -Wall -Werror -Iinclude

%.o: %.c $(HEADERS)
	$(CC) $(CFLAG) -c $< -o $@

main: $(OBJ)
	$(CC) $(CFLAG) $^ -o $@ $(shell mysql_config --cflags --libs)

run: main
	./main

clean:
	rm -rf src/*.o main
