#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

#include "flags.h"

int main(int argc, char **argv) {
    app = (applicationLayer *)malloc(sizeof(applicationLayer));
    llink = (linkLayer *)malloc(sizeof(linkLayer));

    // Parse commnad line arguments
    parse_flags(argc, argv);

    // Ask app to establish connection
    int fd = llopen(app->port, app->status);  

    if (fd < 0) {
        perror("Failed to establish connection.\n");
        free(app);
        free(llink);
        exit(1);
    }

    //Start file transmission
    file_transmission();

    //End connection
    if(llclose(fd) < 0){
        perror("Failed to close connection.\n");
        free(app);
        free(llink);
        exit(1);
    }

    free(app);
    free(llink);

    return 0;
}