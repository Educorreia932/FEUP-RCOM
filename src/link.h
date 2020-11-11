#include "state_machine.h"
#include <string.h>
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

#define SEQUENCE_MASK_S 0x40
#define SEQUENCE_MASK_R 0x80

#define BAUDRATE B38400

typedef struct {
    int baudrate;                  /* Velocidade de transmissão */
    unsigned int sequenceNumber;   /* Número de sequência da trama: 0, 1 */
    unsigned int timeout;          /* Valor do temporizador: 1 s */
    unsigned int numTransmissions; /* Número de tentativas em caso de falha*/
} linkLayer;

linkLayer* llink;

int establish_connection(char* port, enum Status status);
int finish_connection(int fd, enum Status status);
int read_info_frame(int fd, unsigned char** data_field);
int write_info_frame(int fd, unsigned char* packet, int length);