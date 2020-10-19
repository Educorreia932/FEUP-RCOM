
enum Control{
    data,
    start,
    end
};

enum Status{
    TRANSMITTER,
    RECEIVER
};

struct applicationLayer
{
    int fileDescriptor; /*Descritor correspondente à porta série*/
    int status;     /*TRANSMITTER | RECEIVER*/
};

//TODO: Definir estrutura dos pacotes 

int llopen(int port, enum Status stat);
int llread(int fd, char * buffer);
int llwrite(int fd, char * buffer, int length);
int llclose(int fd);