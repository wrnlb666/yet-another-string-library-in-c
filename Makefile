CC = gcc
CFLAG = -g -Wall -Wextra -Warray-bounds=2 -std=gnu11 -pedantic
OBJ = yasli.o


.PHONY: yasli example

yasli: yasli.c yasli.h
	$(CC) $(CFLAG) -fsanitize=leak,bounds,address $< -c

example: example.c yasli.o
	$(CC) $(CFLAG) -fsanitize=leak,bounds,address $(OBJ) $< -o $@

main: main.c yasli.o
	$(CC) $(CFLAG) -fsanitize=leak,bounds,address $(OBJ) $< -o $@
	
yasligc: yasli.c yasli.h
	$(CC) $(CFLAG) -D USE_GC -fsanitize=leak,bounds,address $< -c

examplegc: example.c yasli.o
	$(CC) $(CFLAG) -fsanitize=leak,bounds,address $(OBJ) $< -o example -lgc

yasliwin: yasli.c yasli.h
	$(CC) -Os -Wall -Wextra -std=gnu11 -pedantic -fPIC -shared -static -D USE_GC str.c -o libstr.dll -lgc

yaslili: yasli.c yasli.h
	$(CC) -Os -Wall -Wextra -std=gnu11 -pedantic -fPIC -shared -D USE_GC str.c -o libstr.so -lgc
