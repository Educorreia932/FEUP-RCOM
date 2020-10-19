
#include "app.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct applicationLayer app;

int llopen(char * port, enum Status stat)
{
    int fd = open(port, O_RDWR | O_NOCTTY);

    if (fd < 0) {
		perror(port);
		return -1;
	}

    app.fileDescriptor = fd;
    app.status = stat;

    //TODO: Send start command
    //llwrite(fd, packet, len);

    return fd;
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