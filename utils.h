#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 256

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