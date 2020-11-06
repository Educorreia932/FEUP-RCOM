#include "link.h"

// Control Packet
#define T_FILESIZE 0x00 // T1
#define T_FILENAME 0x01 // T2

#define MAX_CHUNK_SIZE 65536

enum Control {
    data = 1,
    start = 2,
    end = 3
};

typedef struct {
    char port[20];      /* Dispositivo /dev/ttySx, x = 0, 1 */
    int fileDescriptor; /* Descritor correspondente à porta série */
    enum Status status; /* TRANSMITTER | RECEIVER */
    char filename[256];
    int sequence_number;
    int chunk_size;
} applicationLayer;

applicationLayer* app;

int file_transmission();
int llopen(char* port, enum Status status); //TODO: porta devia ser int ???
int llwrite(int fd, unsigned char* buffer, int length);
int llclose(int fd);