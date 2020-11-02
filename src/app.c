
#include "app.h"

#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

FILE* fp;
struct stat st;

int control_packet(enum Control status, char* filename, int filesize, unsigned char** packet) {
    int L1 = ceil(log(filesize) / log(2) / 8);
    size_t L2 = strlen(filename);

    int packet_size = 3 + L1 + 2 + L2; // Number of bytes needed for packet

    *packet = NULL;
    *packet = (unsigned char*) malloc(packet_size);

    (*packet)[0] = status;     // C
    (*packet)[1] = T_FILESIZE; // T1
    (*packet)[2] = L1;         // L1

    int c;

    for (c = 3; c < L1 + 3; c++)
        (*packet)[c] = (filesize >> 8 * (c - 3)) & (0xFF); // V1

    (*packet)[c++] = T_FILENAME; // T2
    (*packet)[c++] = L2;         // L2

    int l = c;
    for (; c < L2 + l; c++)
        (*packet)[c] = filename[c - l];

    return packet_size;
}

int data_packet(unsigned char* data_field, int packet_size, unsigned char** packet) {
    int L1 = packet_size & 0xFF;
    int L2 = packet_size >> 8;

    int length = 4 + packet_size;

    *packet = NULL;
    *packet = (unsigned char*) malloc(length);

    (*packet)[0] = data;
    (*packet)[1] = app->sequence_number;
    (*packet)[2] = L2;
    (*packet)[3] = L1;

    memcpy(*packet + 4, data_field, packet_size);

    return length;
}

struct stat open_file(char* filename) {
    // Open file
    if (app->status == TRANSMITTER) 
        fp = fopen(filename, "r"); //Open for reading

    else 
        fp = fopen(filename, "w"); //Open for writing

    if (fp == NULL) {
        perror("Failed to open file.\n");
        exit(1);
    }

    struct stat st;

    if (stat(filename, &st) < 0) {
        perror("Failed stat call.\n");
        exit(1);
    }

    return st;
}

int llopen(char* port, enum Status status) {
    int fd = establish_connection(port, status);
    app->fileDescriptor = fd;
    app->sequence_number = 0;

    return fd;
}

int llclose(int fd) {
    if (fclose(fp) < 0) {
        perror("Failed to close file.\n");
        exit(1);
    }

    return close(fd);
}

int llwrite(int fd, unsigned char* buffer, int length) {
    return write_info_frame(fd, buffer, length);
}

int llread(int fd, unsigned char** buffer) {
    int length = read_info_frame(fd, buffer);

    return length;
}

void file_transmission() {
    // Send control packets and split the file in data packets to send them
    if (app->status == TRANSMITTER) {
        // Start packet
        st = open_file(app->filename); // Open file to send and send control packet

        unsigned char* packet;
        int length = control_packet(start, app->filename, st.st_size, &packet);

        int n = llwrite(app->fileDescriptor, packet, length);
        free(packet);

        if (n < 0) {
            perror("Failed to send start packet.\n");
            exit(1);
        }

        int num_chunks = ceil(st.st_size / (double) CHUNK_SIZE);

        // Data packets
        for (int i = 0; i < num_chunks; i++) {
            unsigned char data_field[CHUNK_SIZE];

            size_t length = fread(data_field, 1, CHUNK_SIZE, fp);

            int packet_size = data_packet(data_field, length, &packet);

            n = llwrite(app->fileDescriptor, packet, packet_size);
            free(packet);

            app->sequence_number = (app->sequence_number + 1) % 255;

            if (n < 0) {
                perror("Failed to send data packet.\n");
                exit(1);
            }
        }

        // End packet
        int packet_size = control_packet(end, app->filename, st.st_size, &packet);

        n = llwrite(app->fileDescriptor, packet, packet_size);
        free(packet);

        if (n < 0) {
            perror("Failed to send end packet.\n");
            exit(1);
        }
    }

    else if (app->status == RECEIVER) {
        bool transmission_ended = false;
        int L1, L2, L;

        while (!transmission_ended) {
            unsigned char* buffer;
            int length = llread(app->fileDescriptor, &buffer);

            switch (buffer[0]) {
                case start:
                    L1 = buffer[2];
                    int L2_index = 4 + L1;
                    L2 = buffer[L2_index]; // Skip 4 bytes (C, T1, L1 and T2) and V1 field (of size L1)
                    int V2_index = L2_index + 1;

                    char* filename = (char*) malloc(L2);
                    memcpy(filename, buffer + V2_index, L2);

                    st = open_file("../files/pinguim2.gif"); // Open file to send and send control packet

                    break;

                case data:
                    L1 = buffer[3];
                    L2 = buffer[2];
                    L = 256 * L2 + L1;

                    int N = buffer[1]; // Sequence number

                    if (N == app->sequence_number)
                        break;

                    fwrite(buffer + 4, 1, L, fp);
                    app->sequence_number = (app->sequence_number + 1) % 255;

                    break;

                case end: // TODO: Ler packet
                    transmission_ended = true;
                    break;
            }
        }
    }
}
