#include "link.h"

#define FILETOTRANSFER "../files/nose.jpg"

// Control Packet
#define T_FILENAME 0x00 // T1
#define T_FILESIZE 0x01 // T2

#define CHUNK_SIZE 65536

enum Control {
    data = 1,
    start = 2,
    end = 3
};

struct applicationLayer {
    int fileDescriptor; /* Descritor correspondente à porta série */
    enum Status status; /* TRANSMITTER | RECEIVER */
    char* filename;
};

void file_transmission();

int llopen(char *port, enum Status status); //TODO: porta devia ser int ???
int llread(int fd, char *buffer);
int llwrite(int fd, char *buffer, int length);
int llclose(int fd);