#include "link.h"

// Control Packet
#define T_FILESIZE 0x00 // T1
#define T_FILENAME 0x01 // T2

#define MAX_CHUNK_SIZE 65536

enum Control { // T values
    data = 1,
    start = 2,
    end = 3
};

typedef struct {
    char port[20];      /* Dispositivo /dev/ttySx, x = 0, 1 */
    int fileDescriptor; /* Descritor correspondente à porta série */
    enum Status status; /* TRANSMITTER | RECEIVER */
    char filename[256]; /* Nome do ficheiro */
    int sequence_number; /* Numero de sequencia do pacote */
    int chunk_size; /*Tamanho pacote*/
} applicationLayer;

applicationLayer* app;

int file_transmission();
int llopen(char* port, enum Status status); 
int llwrite(int fd, unsigned char* buffer, int length);
int llclose(int fd);