
#include "app.h"

#include <unistd.h>

struct applicationLayer app;

int llopen(int port, enum Status stat)
{
    app.fileDescriptor = port;
    app.status = stat;

    //TODO: Send start command

    return 0;
}

int llclose(int fd)
{
    //TODO: Send end command
    return close(fd); 
}

int llwrite(int fd, char * buffer, int length)
{
    //TODO: preparar pacote
    //TODO: mandar pacote

    return 0;
}

int llread(int fd, char * buffer)
{
    //TODO: receber pacote
    //TODO: Intrepretar pacote
    return 0;
}