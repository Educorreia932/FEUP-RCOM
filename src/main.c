#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

#include "app.h"

int main(int argc, char **argv) {
    // Verify arguments
    // TODO: Move to another file and make a more robust verification
    if ((argc < 3) ||
        ((strcmp("/dev/ttyS10", argv[2]) != 0) &&
         (strcmp("/dev/ttyS11", argv[2]) != 0)) ||
        ((strcmp("-c", argv[1]) != 0) && (strcmp("-s", argv[1]) != 0))) {
        printf(
            "Usage:\tnserial SerialPort status(-s/-c)\n\tex: nserial /dev/ttyS1 "
            "-c\n");
        exit(1);
    }

    // Ask app to establish connection
    int fd;

    if (strcmp("-c", argv[1]) == 0)
        fd = llopen(argv[2], TRANSMITTER);

    else if (strcmp("-s", argv[1]) == 0)
        fd = llopen(argv[2], RECEIVER);

    if (fd < 0) {
        perror("Failed to establish connection.\n");
        exit(1);
    }

    file_transmission();

    llclose(fd); // TODO: Check for errors

    return 0;
}