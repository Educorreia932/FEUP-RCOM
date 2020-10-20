
#include "app.h"
#include "link.h"

#include <unistd.h>

struct applicationLayer app;
FILE * fp;

int control_packet(enum Control type, char * filename, long int filesize)
{
    if(type == start){
        char packet[5]; // TODO: Size

        packet[0] = type; // C
        
        //TLC1
        packet[1] = T_FILENAME; //T1
        packet[2] = sizeof(filename); //L1 number of bytes
        // TODO: add name //V1
        /*
        int n = L_fileName + 1;

        for (int c = 0; c < sizeof(filename); c++){
            packet[n++] = filename[c];
        }
        */

        //TLC2
        /*
        packet[n++] = T_FILESIZE; 
        packet[n++] = sizeof(filename);
        */
        
    }
}

int open_file(char * filename)
{    
    //Open file 
    if(app.status == TRANSMITTER)
        fp = fopen(filename, "r"); //Open for reading
    else fp = fopen(filename, "w"); //Open for writing

    if(fp == NULL){
        perror("Failed to open file.\n");
        exit(1);
    }
    
    if(app.status == TRANSMITTER)
    {
        //Get size
        fseek(fp, 0L, SEEK_END);
        long int filesize = ftell(fp); // filesize is in bytes
        
        rewind(fp); // Go back to beggining

        //TODO: control_packet(start, filename, filesize);
    }

    return 0;
}

int llopen(char * port, enum Status stat)
{
    app.status = stat;
    
    int fd = open_port(port);
    app.fileDescriptor = fd;

    //TODO: Send start command
    open_file(FILETOTRANSFER); //Open file & set TLV values if transmitter

    /* if(app.status == TRANSMITTER){
        TODO: Manda mensagem SET
        TODO: RECEBE mensagem UA
    } */

    return fd;
}

int llclose(int fd)
{
    //TODO: Send end command
    return close(fd); 
}

int llwrite(int fd, char * buffer, int length)
{
    //TODO: preparar pacote
    //TODO: mandar pacote

    return 0;
}

int llread(int fd, char * buffer)
{
    //TODO: receber pacote
    //TODO: Intrepretar pacote
    return 0;
}