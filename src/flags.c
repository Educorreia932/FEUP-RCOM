#include "flags.h"

void parse_flags(int argc, char** argv) {
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

        // Baud Rate
        else if (!strcmp(argv[i], "-b") || !strcmp(argv[i], "--baudrate"))
            llink->baudrate = atoi(argv[i + 1]);

        // Timeout
        else if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "--timeout"))
            llink->timeout = atoi(argv[i + 1]);

        // Number of transmissions
        else if (!strcmp(argv[i], "-n") || !strcmp(argv[i], "--num_transmissions"))
            llink->numTransmissions = atoi(argv[i + 1]);

        // Alarm time (in seconds)
        else if (!strcmp(argv[i], "-n") || !strcmp(argv[i], "--alarm"))
            llink->numTransmissions = atoi(argv[i + 1]);

        // File to transfer
        else if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--file"))
            strcpy(app->filename, argv[i + 1]);
    }
}