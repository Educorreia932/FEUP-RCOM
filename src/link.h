#include "state_machine.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define BAUDRATE B38400
#define TIMEOUT 3
#define NUM_TRANSMITIONS 3
#define MAX_SIZE 255


struct linkLayer {
	char port[20]; /* Dispositivo /dev/ttySx, x = 0, 1 */
	int baudRate; /* Velocidade de transmissão */
	unsigned int sequenceNumber; /* Número de sequência da trama: 0, 1 */
	unsigned int timeout; /* Valor do temporizador: 1 s */
	unsigned int numTransmissions; /* Número de tentativas em caso de falha*/
	char frame[MAX_SIZE]; /* Trama */
};

int establish_connection(char * port, enum Status stat);