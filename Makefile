CC = gcc
CFLAG = -Wall -Wextra -std=gnu11 -pedantic
OBJ = str.o


.PHONY: str example

str: str.c str.h
	$(CC) $(CFLAG) -fsanitize=leak,bounds,address $< -c

example: example.c str.o
	$(CC) $(CFLAG) -fsanitize=leak,bounds,address $(OBJ) $< -o $@

strgc: str.c str.h
	$(CC) $(CFLAG) -D USE_GC -fsanitize=leak,bounds,address $< -c

examplegc: example.c str.o
	$(CC) $(CFLAG) -fsanitize=leak,bounds,address $(OBJ) $< -o example -lgc
