#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_LEN 256
#define h_addr h_addr_list[0] // The first address in h_addr_list.
#define SERVER_PORT 21

/**
 * Struct used to store fields passed in arguments
*/
struct url {
    char user[50];
    char password[50];
    char filepath[100];
    char host[50];
};

/**
 * Parses arguments of a "ftp://[<user>:<password>@]<host>/<file-path>" string 
*/
int parse_url(char* arguments, struct url* url);

int get_port(char* str);

int create_socket(char* ip, int port);

int get_file_size(char* response);

int download_file(int file_size, int data_socket_fd, char* filepath);

void progress_bar(float percentage);

char readFromSocket(FILE* fp);