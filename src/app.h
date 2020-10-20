
#include "link.h"

#define FILETOTRANSFER "../files/pinguin.gif"

//Control Packet
#define T_FILENAME 0x00; //T1
#define T_FILESIZE 0x01; //T2

enum Control{
    data,
    start,
    end
};

struct applicationLayer
{
    int fileDescriptor; /*Descritor correspondente à porta série*/
    int status;     /*TRANSMITTER | RECEIVER*/
};


int llopen(char * port, enum Status stat); //porta devia ser int ???
int llread(int fd, char * buffer);
int llwrite(int fd, char * buffer, int length);
int llclose(int fd);