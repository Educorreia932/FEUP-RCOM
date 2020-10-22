
#include "app.h"

#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

struct applicationLayer app;
FILE *fp;
struct stat st;

char *start_end_packet(enum Control status, char *filename, long int filesize) {
    char *packet;

    size_t L1 = strlen(filename);
    int L2 = ceil(filesize / 8);

    int packet_size = 3 + L1 + 2 + L2; // Number of bytes needed for packet

    packet = malloc(packet_size);

    packet[0] = status;
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

char *data_packet(char *data_field, int length) {
    char *packet;

    // TODO: Change later
    int L1 = 0xFF;
    int L2 = 0xFF;

    int packet_size = 4 + length;

    packet = malloc(packet_size);
    packet[0] = data;
    packet[1] = 0; // TODO: Change later
    packet[2] = L1;
    packet[3] = L2;

    packet += 4;

    memmove(packet, data_field, length);

    return packet;
}

void file_transmission() {
    // Send control packets and split the file in data packets to send them
    if (app.status == TRANSMITTER) {
        // Start packet
        char *packet = start_end_packet(start, app.filename, st.st_size);
        llwrite(app.fileDescriptor, packet, sizeof(packet)); //Mandar junto com as outras ?????????

        int num_chunks = ceil(st.st_size / (double)CHUNK_SIZE);

        // Data packets
        for (int i = 0; i < num_chunks; i++) {
            char *data_field = (char *) malloc(CHUNK_SIZE);

            fseek(fp, 0, SEEK_CUR);
            size_t length = fread(data_field, 1, CHUNK_SIZE, fp);

            packet = data_packet(data_field, length);

            llwrite(app.fileDescriptor, packet, length);
        }

        // End packet
        packet = start_end_packet(end, app.filename, st.st_size);
        llwrite(app.fileDescriptor, packet, sizeof(packet));
    }
    
    else if (app.status == RECEIVER) { //TODO: separate function?? 
        bool transmission_ended = false;

        while (!transmission_ended) {

            char* buf;
            llread(app.fileDescriptor, buf);

            //TODO: check if end packet arrived  

            transmission_ended = true; //TODO: if receive End packet 
        }
    }
}

struct stat open_file(char *filename) {
    // Open file
    if (app.status == TRANSMITTER) {
        fp = fopen(filename, "r"); //Open for reading
    }

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
    app.status = status;
    
    int fd = establish_connection(port, status);
    app.fileDescriptor = fd;

    st = open_file(FILETOTRANSFER); // Open file to send and send control packet

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
    //send_information_frame(fd, A_EM_CMD, C_I, buffer, length); //TODO: Devia ser o linklayer a chamar esta funçao

    return write_info_frame(fd, buffer, length);
}

int llread(int fd, char *buffer) {
    sleep(1);
    buffer = receive_info_frame(fd);
    fwrite(buffer, 1, 4096, fp);

    return 0;
}