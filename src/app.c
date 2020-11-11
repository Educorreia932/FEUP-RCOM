
#include "app.h"

#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>

FILE* fp;

/**
 * Creates a control packet based on file.
 */
int control_packet(enum Control status, char* filename, int filesize, unsigned char** packet) {
    int L1 = ceil(log(filesize) / log(2) / 8); // Calculates L1
    size_t L2 = strlen(filename); // Calculates L2

    // Allocs memory for packet
    int packet_size = 3 + L1 + 2 + L2; // Calculates size of packet (C, T1, L1, V1, T2, L2, V2)
    *packet = NULL;
    *packet = (unsigned char*) malloc(packet_size);

    // Creates packet
    int c_ind = 0, t1_ind = 1, l1_ind = 2, v1_ind = 3; // indexes
    (*packet)[c_ind] = status;     // C
    (*packet)[t1_ind] = T_FILESIZE; // T1
    (*packet)[l1_ind] = L1;         // L1

    // V1
    int c;

    for (c = v1_ind; c < (L1 + v1_ind); c++)
        (*packet)[c] = (filesize >> 8 * (c - v1_ind)) & (0xFF); // V1
    
    (*packet)[c++] = T_FILENAME; // T2
    (*packet)[c++] = L2;         // L2

    // V2
    int v2_ind = c;

    for (; c < L2 + v2_ind; c++)
        (*packet)[c] = filename[c - v2_ind];

    return packet_size;
}

/**
 * Create a data packet. 
 * Returns the length of the created packet.
 */
int data_packet(unsigned char* data_field, int data_size, unsigned char** packet) {
    int L1 = data_size & 0xFF; // Calculates L1
    int L2 = data_size >> 8; // Calculates L2

    // Allocs memory for packet
    int packet_length = 4 + data_size; // Length of result packet will be data size+ 4 bytes (C, N, L1, L2)
    *packet = NULL;
    *packet = (unsigned char*) malloc(packet_length);

    // Creates packets
    int C_ind = 0, N_ind = 1, L2_ind = 2, L1_ind = 3, data_ind = 4; // indexes
    (*packet)[C_ind] = data;
    (*packet)[N_ind] = app->sequence_number;
    (*packet)[L2_ind] = L2;
    (*packet)[L1_ind] = L1;
    memcpy(*packet + data_ind, data_field, data_size); // Adds data to packet

    return packet_length;
}

/**
 * Opens file with name filename.
 * If transmitter, opens for reading. Otherwise it opens for writing.
 * Returns a struct which contains the size of the file.
 */
struct stat open_file(char* filename) {
    // Open file
    if (app->status == TRANSMITTER) 
        fp = fopen(filename, "r"); //Open for reading

    else // RECEIVER
        fp = fopen(filename, "w"); //Open for writing

    if (fp == NULL) {
        perror("Failed to open file.\n");
        exit(1);
    }

    // Stat call to get filesize
    struct stat st;
    if (stat(filename, &st) < 0) {
        perror("Failed stat call.\n");
        exit(1);
    }

    return st;
}

/**
 * Establishes an connection.
 */
int llopen(char* port, enum Status status) {
    // Tells link layer to establish a connection 
    int fd = establish_connection(port, status);
    
    //Update struct values
    app->fileDescriptor = fd;
    app->sequence_number = 0;

    return fd;
}

/**
 * Closes file and finishes connection.
 */
int llclose(int fd) {
    // Close file
    if (fclose(fp) < 0) {
        perror("Failed to close file.\n");
        exit(1);
    }

    // Tells link layer to finish connection
    if(finish_connection(fd, app->status) < 0)
        return -1;

    return close(fd);
}

/**
 * Sends packets to link layer.
 */
int llwrite(int fd, unsigned char* buffer, int length) {
    return write_info_frame(fd, buffer, length);
}

/**
 * Receives packets from link layer.
 */
int llread(int fd, unsigned char** buffer) {
    return read_info_frame(fd, buffer); 
}

/**
 * Prints a progress bar
 * Showing the current transmission progress
 */
void progress_bar(float progress) { 
    int length = 50;

    for (int i = 0; i <= length * progress; i++)
        printf("\u2588"); // █

    for (int i = length * progress; i < length; i++)
        printf("\u2581"); // ▁

    printf("  %.2f%% Complete\n", progress * 100);
} 

/**
 * Sends file in case of transmitter.
 * Receives file in case of receiver.
 */
int file_transmission() {
    struct stat st;

    // Transmitter
    if (app->status == TRANSMITTER) { 
        st = open_file(app->filename); // Open file to send 

        // Creates Start Packet
        unsigned char* packet;
        int packet_size = control_packet(start, app->filename, st.st_size, &packet);

        // Sends Start packet
        if(llwrite(app->fileDescriptor, packet, packet_size) < 0){
            free(packet);
            perror("Failed to send start packet.\n");
            exit(1);
        }
        
        free(packet);

        char filearray[st.st_size];
        fread(filearray, 1, st.st_size, fp); // Reads from file and stores in data_field

        // Calculate the number of chunks in which the file will be split
        int num_chunks = ceil(st.st_size / (double) app->chunk_size); 

        // Creates data packets
        size_t length = app->chunk_size;
        int index = 0;

        for (int i = 0; i < num_chunks; i++) {
            unsigned char data_field[app->chunk_size];
            if(i == num_chunks - 1) 
                length = st.st_size - (length * i) ;
            
            packet_size = data_packet(filearray + index, length, &packet); // Prepares a data packet 
            index += app->chunk_size;

            //Sends data packet
            if(llwrite(app->fileDescriptor, packet, packet_size) < 0){
                free(packet);    
                perror("Failed to send data packet.\n");
                exit(1);
            }

            free(packet);

            // Update sequence number
            app->sequence_number = (app->sequence_number + 1) % 255;
        }

        // Creates End Packet
        packet_size = control_packet(end, app->filename, st.st_size, &packet);

        
        // Sends End packet
        if(llwrite(app->fileDescriptor, packet, packet_size) < 0){
            free(packet);
            perror("Failed to send end packet.\n");
            exit(1);
        }

        
        
        free(packet);
    }

    // Receiver
    else if (app->status == RECEIVER) {
        int L1_index, L2_index;
        int L1, L2;

        unsigned char* file_array;
        int filesize = 0, file_index = 0;

        bool transmission_ended = false;
        while (!transmission_ended) {
            // Receive packet
            unsigned char* buffer;

            if(llread(app->fileDescriptor, &buffer) == 0)
                continue; // Empty packet
    
            switch (buffer[0]) {
                case start: // Received Start Packet
                    //Parsing Data
                    // L1
                    L1_index = 2;
                    L1 = buffer[L1_index];

                    // L2
                    L2_index = 4 + L1; // Skip 4 bytes (C, T1, L1 and T2) and V1 field (of size L1)
                    L2 = buffer[L2_index]; 

                    // V1 - File Size
                    int V1_index = L1_index + 1;
                    for (int i = 0; i < L1; i++)
                        filesize += buffer[V1_index + i] * pow(256, i);
                    
                    // V2 - File Name
                    int V2_index = L2_index + 1;
                    char* filename = (char*) malloc(L2);
                    memcpy(filename, buffer + V2_index, L2);

                    // Open file 
                    // st = open_file(filename); 
                    st = open_file("../files/received.gif"); 
                    // Create file array where data received will be written to
                    file_array = (unsigned char*) malloc(filesize);

                    free(filename);

                    break;

                case data: // Received Data Packet
                    L1_index = 3;
                    L2_index = 2;
                    L1 = buffer[L1_index]; // L1
                    L2 = buffer[L2_index]; // L2
                    
                    int K = 256 * L2 + L1; // K

                    //Sequence number
                    int N_index = 1;
                    int N = buffer[N_index]; // N - Sequence number

                    if (N > app->sequence_number)
                        break; // Ignore repeated packet

                    app->sequence_number = (app->sequence_number + 1) % 255; // Update sequence number

                    // Save data to file array
                    int data_index = 4;
                    memcpy(file_array + file_index, buffer + data_index, K);
                    file_index += app->chunk_size;

                    // Update progress bar
                    float progress = file_index / (float) filesize;

                    if (progress > 1)
                        progress = 1;

                    progress_bar(progress);

                    break;

                case end: // Received End Packet
                    // Writes contents of array to file 
                    fwrite(file_array, 1, filesize, fp); 
                    // Ends cycle
                    transmission_ended = true;

                    free(file_array);

                    break;
            }
        }
    }

    return 0;
}
