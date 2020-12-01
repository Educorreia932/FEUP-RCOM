#include "utils.h"

int main(int argc, char** argv) {

    if(argc != 2){
        printf("Usage download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(1);
    }

    // Parse & store arguments
    struct fields fields;
    parse_fields(argv[1], &fields);
    print_fields(fields);

    // Host
    struct hostent* h;

    if ((h = gethostbyname(fields.host)) == NULL) {
        herror("gethostbyname");
        exit(1);
    }

    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n", inet_ntoa(*((struct in_addr*) h->h_addr)));

    return 0;
}