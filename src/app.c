
#include "app.h"

#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

FILE *fp;
struct stat st;

int control_packet(enum Control status, char *filename, long int filesize, char* packet) {
    int L1 = ceil(log(filesize) / log(2) / 8);
    size_t L2 = strlen(filename);

    int packet_size = 3 + L1 + 2 + L2; // Number of bytes needed for packet

    int n = realloc(packet, packet_size);

    packet[0] = status;
    packet[1] = T_FILENAME;
    packet[2] = L1;

    int c;

    for (c = 3; c < L1 + 3; c++)
        packet[c] = filename[c - 3];

    packet[++c] = T_FILESIZE;
    packet[++c] = L2;

    return packet_size;
}

int data_packet(char *data_field, int packet_size) {
    char *packet;

    int L1 = (packet_size & 0xFF00) >> 8;
    int L2 = packet_size & 0xFF;

    int length = 4 + packet_size;

    packet = malloc(packet_size);
    packet[0] = data;
    packet[1] = 0; // TODO: Change later
    packet[2] = L1;
    packet[3] = L2;

    memcpy(packet + 4, data_field, packet_size);
    memcpy(data_field, packet, length);

    return length;
}

struct stat open_file(char *filename) {
    // Open file
    if (app->status == TRANSMITTER) {
        fp = fopen(filename, "r"); //Open for reading
    }

    else {
        // TODO: This is only needed when using SOCAT on the same PC
        char copy_filename[255] = "../files/teste_copia.png";
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

    app->filename = filename;

    return st;
}

void file_transmission() {
    // Send control packets and split the file in data packets to send them
    if (app->status == TRANSMITTER) {
        // Start packet
        st = open_file(FILETOTRANSFER); // Open file to send and send control packet

        char* packet = malloc(1);
        int length = control_packet(start, app->filename, st.st_size, packet);
        int n = llwrite(app->fileDescriptor, packet, length);
        
        if(n < 0){
            perror("Failed to send start packet.\n");
            exit(1);
        }

        int num_chunks = ceil(st.st_size / (double)CHUNK_SIZE);

        // Data packets
        for (int i = 0; i < num_chunks; i++) {
            char *data_field = (char *) malloc(CHUNK_SIZE);

            size_t length = fread(data_field, 1, CHUNK_SIZE, fp);

            int packet_size = data_packet(data_field, length);

            n = llwrite(app->fileDescriptor, data_field, packet_size);

            if (n < 0){
                perror("Failed to send data packet.\n");
                exit(1);
            }
        }

        // End packet
        int packet_size = control_packet(end, app->filename, st.st_size, packet);
        n = llwrite(app->fileDescriptor, packet, packet_size);
        
        if(n < 0){
            perror("Failed to send end packet.\n");
            exit(1);
        }
    }
    
    else if (app->status == RECEIVER) {
        bool transmission_ended = false;
        int L1, L2, L;

        while (!transmission_ended) {
            char* buffer = (char*) malloc(MAX_SIZE);
            int length = llread(app->fileDescriptor, buffer);

            switch (buffer[0]) {
                case start:
                    L1 = buffer[2];

                    char* filename;
                    
                    st = open_file(FILETOTRANSFER); // Open file to send and send control packet

                    break;

                case data:
                    L1 = buffer[3];
                    L2 = buffer[2];
                    L = 256 * L2 + L1;

                    fwrite(buffer + 4, 1, L, fp);
                    break;

                case end:
                    transmission_ended = true; 
                    break;
            }
        }
    }
}

int llopen(char* port, enum Status status) {
    app->status = status;
    
    int fd = establish_connection(port, status);
    app->fileDescriptor = fd;

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
    return write_info_frame(fd, buffer, length);
}

int llread(int fd, char *buffer) {
    int length = read_info_frame(fd, buffer);

    return length;
}