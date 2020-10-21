
#include "app.h"

#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

struct applicationLayer app;
FILE *fp;

char *start_packet(enum Control type, char *filename, long int filesize) {
    char *packet;

    size_t L1 = strlen(filename);
    int L2 = ceil(filesize / 8);

    int packet_size = 3 + L1 + 2 + L2; // Number of bytes

    packet = malloc(packet_size);

    packet[0] = type;
    packet[1] = T_FILENAME;
    packet[2] = L1;

    int c;

    for (c = 3; c < L1 + 3; c++)
        packet[c] = filename[c - 3];

    packet[c++] = T_FILESIZE;
    packet[c++] = L2;

    for (c = 3; c < L1 + 3; c++) {
        int octet = (filesize >>= 8);

        packet[c] = filename[c - 3];
    }

    return packet;
}

char *end_packet() {
    char *packet = malloc(1);
    packet[0] = end;

    return packet;
}

int open_file(char *filename) {
    // Open file
    if (app.status == TRANSMITTER)
        fp = fopen(filename, "r"); //Open for reading

    else {
        // TODO: This is only needed when using SOCAT on the same PC
        char copy_filename[255] = "../files/pinguim_copia.gif";
        fp = fopen(copy_filename, "w"); //Open for writing
    }

    if (fp == NULL) {
        perror("Failed to open file.\n");
        exit(1);
    }

    if (app.status == TRANSMITTER) // If transmitter, need to send Start Packet
    {
        struct stat st;

        if (stat(filename, &st) < 0) {
            perror("Failed stat call.\n");
            exit(1);
        }

        int filesize = st.st_size; // File size

        char *packet = start_packet(start, filename, filesize);

        llwrite(app.fileDescriptor, packet, sizeof(packet));
    }

    return 0;
}

int llopen(char *port, enum Status stat) {
    app.status = stat;

    int fd = establish_connection(port, stat);
    app.fileDescriptor = fd;

    open_file(FILETOTRANSFER); //Open file & send Start packet

    return fd;
}

int llclose(int fd) {
    char *packet = end_packet(); // Check if it's necessary to fill filesize and filename
    llwrite(fd, packet, sizeof(packet));

    if (fclose(fp) < 0) {
        perror("Failed to close file.\n");
        exit(1);
    }

    return close(fd);
}

int llwrite(int fd, char *buffer, int length) {
    //TODO: preparar pacote
    //TODO: mandar pacote

    return 0;
}

int llread(int fd, char *buffer) {
    //TODO: receber pacote
    //TODO: Intrepretar pacote
    return 0;
}