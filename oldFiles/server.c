/*Non-Canonical Input Processing*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "utils.h"

#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

int main(int argc, char** argv) {
	int fd, c, res;
	struct termios oldtio, newtio;
	char buf[255];
	state current_state = START;
	int fields[5] = {FLAG, A_EM_CMD, C_SET, A_EM_CMD ^ C_SET, FLAG};
	
	struct linkLayer llink;

	strcpy(llink.port, argv[1]);
	llink.baudRate = BAUDRATE;
	llink.sequenceNumber = 0;
	llink.timeout = TIMEOUT;
	llink.numTransmissions = NUM_TRANSMITIONS;

	if ((argc < 2) || ((strcmp("/dev/ttyS10", llink.port) != 0) &&
						(strcmp("/dev/ttyS11", llink.port) != 0))) {
		printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
		exit(1);
	}

	/*
	Open serial port device for reading and writing and not as controlling tty
	because we don't want to get killed if linenoise sends CTRL-C.
	*/

	fd = open(argv[1], O_RDWR | O_NOCTTY);
	if (fd < 0) {
	perror(argv[1]);
	exit(-1);
	}

	if (tcgetattr(fd, &oldtio) == -1) { /* save current port settings */
	perror("tcgetattr");
	exit(-1);
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
	newtio.c_cc[VMIN] = 1;  /* blocking read until 5 chars received */

	/*
	VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
	leitura do(s) pr�ximo(s) caracter(es)
	*/

	tcflush(fd, TCIOFLUSH);

	if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	printf("New termios structure set\n");

	res = read(fd, buf, 1);  // received message

	if (res == -1) {
		perror("Failed to read.");
		exit(1);
	}

	// State machine
	if (*buf == fields[current_state]) 
		current_state++;

	else if (*buf == FLAG) // FLAG_RCV
		current_state = FLAG_RCV;

	else // OTHER_RCV
		current_state = START;

	if (current_state == STOP) {
		printf("Received SET msg\n");
	}

	// Write acknowledgment message to serial port
	res = send_frame(fd, A_RC_RESP, C_UA);

	if (res == -1) {
		perror("Failed to send UA msg.\n");
		exit(1);
	}

	printf("Sent message UA.\n");

	tcsetattr(fd, TCSANOW, &oldtio);
	close(fd);
	return 0;
}
