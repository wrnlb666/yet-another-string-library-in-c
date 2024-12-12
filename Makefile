CC = gcc
CFLAG = -g -Wall -Wextra -Warray-bounds=2 -std=gnu11 -pedantic
OBJ = yasli.o


.PHONY: yasli example

yasli: yasli.c yasli.h
	$(CC) $(CFLAG)  $< -c

example: example.c yasli.o
	$(CC) $(CFLAG)  $(OBJ) $< -o $@

main: main.c yasli.o
	$(CC) $(CFLAG)  $(OBJ) $< -o $@
	
yasligc: yasli.c yasli.h
	$(CC) $(CFLAG) -D USE_GC  $< -c

examplegc: example.c yasli.o
	$(CC) $(CFLAG)  $(OBJ) $< -o example -lgc

yasliwin: yasli.c yasli.h
	$(CC) -Os -Wall -Wextra -std=gnu11 -pedantic -fPIC -shared -static -D USE_GC yasli.c -o libyasli.dll -lgc

yaslili: yasli.c yasli.h
	$(CC) -Os -Wall -Wextra -std=gnu11 -pedantic -fPIC -shared -D USE_GC yasli.c -o libyasli.so -lgc
