/*Non-Canonical Input Processing*/

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

#include "utils.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

bool flag = true;
int alarm_counter = true;

void atende() { // atende alarme
	printf("alarme # %d\n", alarm_counter);
	flag = true;
	alarm_counter++;
}

int main(int argc, char** argv) {
	int fd, c, res;
	struct termios oldtio, newtio;
	char buf[255];
	int i, sum = 0, speed = 0;
	state current_state = START;
	int fields[5] = {FLAG, A_RC_RESP, C_UA, A_RC_RESP ^ C_UA, FLAG};

	struct linkLayer link;

	strcpy(link.port, argv[1]);
	link.baudRate = BAUDRATE;
	link.sequenceNumber = 0;
	link.timeout = TIMEOUT;
	link.numTransmissions = NUM_TRANSMITIONS;

	if ((argc < 2) || ((strcmp("/dev/ttyS10", argv[1]) != 0) &&
						(strcmp("/dev/ttyS11", argv[1]) != 0))) {
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

	/* Save current port settings */
	if (tcgetattr(fd, &oldtio) == -1) { 
		perror("tcgetattr");
		exit(-1);
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* Set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME] = 0; /* Inter-character timer unused */
	newtio.c_cc[VMIN] = 1;  /* Blocking read until 5 chars received */

	/*
	VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
	leitura do(s) próximo(s) caracter(es)
	*/

	tcflush(fd, TCIOFLUSH);

	if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	printf("New termios structure set\n");

	//----------------------------

	struct sigaction action;

	action.sa_handler = &atende;
	sigemptyset (&action.sa_mask);
	action.sa_flags = 0;

	sigaction(SIGALRM, &action, NULL);  // Installs co-routine that attends interruption

	char msg[5];

	while (alarm_counter <= link.numTransmissions) { /* loop for input */
		if (flag) {
			alarm(link.timeout);  // Activactes 3 second alarm
			flag = false;

			int n = send_frame(fd, A_EM_CMD, C_SET);  // Send SET msg

			if (n == -1) {
				perror("Failed to send SET message.");
				exit(1);
			}

			printf("Sent SET message. \n");
		}

		res = read(fd, buf, 1); // Receive msg

		if (res == -1) {
			// Read was not interrupted
			if (errno != EINTR) {
				perror("Failed to read.");
				exit(1);		
			}
		}

		// State machine
		if (*buf == fields[current_state]) 
			current_state++;

		else if (*buf == FLAG) // FLAG_RCV
			current_state = FLAG_RCV;

		else // OTHER_RCV
			current_state = START;

		if (current_state == STOP) {
			alarm(0);  // Pending alarm is canceled
			printf("Received UA msg\n");
			break;
		}
	}

	if (alarm_counter > 3)
		printf("Failed to receive UA msg.");

	//----------------------------

	sleep(1);

	if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	close(fd);
	return 0;
}
