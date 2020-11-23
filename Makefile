# Compiler and linker
CC = gcc

# Flags
CFLAGS = -Wall -g

main: 
	@gcc ${CFLAGS} -o download main.c utils.c -lm
