NAME_SERVER = noncanonical
SRC_SERVER = noncanonical.c

NAME_CLIENT = writenoncanonical
SRC_CLIENT = writenoncanonical.c

# Compiler and linker
CC = gcc

# Flags
CFLAGS = -Wall

all: 
	options rcom

options:
	@echo RCOM build options:
	@echo "CFLAGS = ${CFLAGS}"
	@echo "CC = ${CC}"

.c.o:
	${CC} -c ${CFLAGS} $<

rcom: 
	${CC} ${SRC_SERVER} -o ${NAME_SERVER}
	${CC} ${SRC_CLIENT} -o ${NAME_CLIENT}

clean:
	rm -f ${NAME_SERVER} ${OBJ_SERVER} ${NAME_CLIENT} ${OBJ_CLIENT}

.PHONY: all options clean rcom