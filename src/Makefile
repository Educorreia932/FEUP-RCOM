# Compiler and linker
CC = gcc

# Flags
CFLAGS = -Wall -g

main: 
	@gcc ${CFLAGS} -o main app.c link.c main.c state_machine.c flags.c log.c -lm

clean:
	@rm -f main