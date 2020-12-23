#include "app.h"

void parse_flags(int argc, char *argv[]);

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

void parse_flags(int argc, char** argv) {
    app->chunk_size = MAX_CHUNK_SIZE;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], ""))
            continue;

        // Serial Port
        if (!strncmp(argv[i], "/dev/ttyS", 9))
            strcpy(app->port, argv[i]);

        // Status
        else if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--client"))
            app->status = TRANSMITTER;

        else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--server"))
            app->status = RECEIVER;

        // Timeout
        else if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "--timeout"))
            llink->timeout = atoi(argv[i + 1]);

        // Number of transmissions
        else if (!strcmp(argv[i], "-n") || !strcmp(argv[i], "--num_transmissions"))
            llink->numTransmissions = atoi(argv[i + 1]);

        // File to transfer
        else if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--file"))
            strcpy(app->filename, argv[i + 1]);

        // Chunk size
        else if (!strcmp(argv[i], "--chunk_size"))
            app->chunk_size = atoi(argv[i + 1]);
    }
}