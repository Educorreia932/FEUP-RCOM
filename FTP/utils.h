#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>

#define MAX_LEN 256
#define h_addr h_addr_list[0] // The first address in h_addr_list.
#define SERVER_PORT 21
/**
 * Struct used to store fileds passes in arguments
*/
struct fields {
    char user[MAX_LEN];
    char password[MAX_LEN];
    char url[MAX_LEN];
    char host[MAX_LEN];
};

/**
 * Parses arguments of a "ftp://[<user>:<password>@]<host>/<url-path>" string 
*/
int parse_fields(char* arguments, struct fields* fields);

void print_fields(struct fields fields);

void parse_file_port(char* str, char** ip, int* port);

int create_socket(char * ip, int port);

int download_file(int data_socket_fd, char* filename);