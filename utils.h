#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>

#define MAX_LEN 256
#define h_addr h_addr_list[0] // The first address in h_addr_list.

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