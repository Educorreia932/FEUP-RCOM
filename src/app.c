
#include "app.h"

#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

struct applicationLayer app;
FILE *fp;
struct stat st;

char *start_packet(char *filename, long int filesize) {
    char *packet;

    size_t L1 = strlen(filename);
    int L2 = ceil(filesize / 8);

    int packet_size = 3 + L1 + 2 + L2; // Number of bytes

    packet = malloc(packet_size);

    packet[0] = start;
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

char* data_packet(char* data_field) {
    char *packet;

    // TODO: Change later
    int L1 = 0xFF;
    int L2 =  0xFF;

    int data_size = sizeof(data_field);
    int packet_size = 4 + data_size;

    packet = malloc(packet_size);
    packet[0] = data;
    packet[1] = 0; // TODO: Change later
    packet[2] = L1;
    packet[3] = L2;

    packet += 4;

    memmove(packet, data_field, data_size);

    return packet;
}

char *end_packet() {
    char *packet = malloc(1);
    packet[0] = end;

    return packet;
}

// Send control packets and split the file in data packets
void send_file() {
    // Start packet
    char *packet = start_packet(app.filename, st.st_size);
    llwrite(app.fileDescriptor, packet, sizeof(packet));

    int num_chunks = ceil(st.st_size / CHUNK_SIZE);

    // Data packets
    for (int i = 0; i < num_chunks; i++) {
        char* data_field = (char*) calloc(CHUNK_SIZE, sizeof(char));

        fseek(fp, 0, SEEK_CUR);
        fread(data_field, 1, CHUNK_SIZE, fp);

        packet = data_packet(data_field);
    }

    //End packet
    packet = end_packet();
    llwrite(app.fileDescriptor, packet, sizeof(packet));
}

struct stat open_file(char *filename) {
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

    struct stat st;

    if (stat(filename, &st) < 0) {
        perror("Failed stat call.\n");
        exit(1);
    }

    app.filename = filename;

    return st;
}

int llopen(char *port, enum Status status) {
    st = open_file(FILETOTRANSFER); // Open file to send and send control packet

    app.status = status;

    int fd = establish_connection(port, status);
    app.fileDescriptor = fd;

    return fd;
}

int llclose(int fd) {
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