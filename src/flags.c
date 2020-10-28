#include "flags.h"

void parse_flags(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], ""))
            continue;

        // Serial Port
        if (!strncmp(argv[i], "/dev/ttyS", 9))
            strcpy(llink->port, argv[i]);

        // Status
        else if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--client"))
            app->status = TRANSMITTER;

        else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--server"))
            app->status = RECEIVER;
    }
}