#include "app.h"

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

int main(int argc, char** argv) {
    
    //Verify arguments
    if ((argc < 3) || ((strcmp("/dev/ttyS10", argv[1]) != 0) && (strcmp("/dev/ttyS11", argv[1]) != 0))
    || ((strcmp("-c", argv[2]) != 0) && (strcmp("-s", argv[2]) != 0))) 
    {
		printf("Usage:\tnserial SerialPort status(-s/-c)\n\tex: nserial /dev/ttyS1 -c\n");
		exit(1);
	}

    //Ask app to establish connection
    int fd;
    if(strcmp("-c", argv[2]) == 0)
        fd = llopen(argv[1], TRANSMITTER);
    else fd = llopen(argv[1], RECEIVER);

    if(fd < 0){
        perror("Failed to establish connection.\n");
        exit(1);
    }
    
    
    //TODO: Ask app to open image and send it or receive it
    //TODO: Receive confirmation that app finished sending/receiving
    

    return 0;
}