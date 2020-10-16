#define FLAG 0x7E

#define A_EM_CMD 0x03
#define A_RC_RESP 0x03
#define A_RC_CMD 0x01
#define A_EM_RESP 0x01

#define C_SET 0x03
#define C_UA 0x07

#define BAUDRATE B38400
#define TIMEOUT 3
#define NUM_TRANSMITIONS 3
#define MAX_SIZE 255

typedef enum {
	START,
	FLAG_RCV,
	A_RCV,
	C_RCV,
	BCC_RCV,
	STOP
} state;

struct applicationLayer {
	int fileDescriptor; /*Descritor correspondente à porta série*/
	int status; /*TRANSMITTER | RECEIVER*/
};

struct linkLayer {
	char port[20]; /* Dispositivo /dev/ttySx, x = 0, 1 */
	int baudRate; /* Velocidade de transmissão */
	unsigned int sequenceNumber; /* Número de sequência da trama: 0, 1 */
	unsigned int timeout; /* Valor do temporizador: 1 s */
	unsigned int numTransmissions; /* Número de tentativas em caso de falha*/
	char frame[MAX_SIZE]; /* Trama */
};

int send_frame(int fd, char a, char c) {
	unsigned char buf[5];

	buf[0] = FLAG;
	buf[1] = a;
	buf[2] = c;
	buf[3] = a ^ c; // BCC
	buf[4] = FLAG;

	return write(fd, buf, 5);
}
