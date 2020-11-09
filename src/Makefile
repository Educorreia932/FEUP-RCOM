# Compiler and linker
CC = gcc

# Flags
CFLAGS = -Wall -g

main: 
	@gcc ${CFLAGS} -o main app.c link.c main.c state_machine.c -lm

clean:
	@rm -f main

initialize:
	@sudo socat -d -d PTY,link=/dev/ttyS0,mode=777 PTY,link=/dev/ttyS1,mode=777

client: clean main 
	@./main -c "/dev/ttyS0" --baudrate B38400 --timeout 5 --num_transmissions 3 --chunk_size 256 --file "../files/pinguim.gif"

server: clean main
	@./main -s "/dev/ttyS1" --baudrate B38400 --timeout 5 --num_transmissions 3 --chunk_size 256