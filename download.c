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
    puts("");

    char * address = inet_ntoa(*((struct in_addr*) h->h_addr));

    //TCP
    int sockfd;
    struct sockaddr_in server_addr;

    /* Server address handling */
    bzero((char*) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(address); /* 32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(SERVER_PORT); /* server TCP port must be network byte ordered */

    /* Open an TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(0);
    }

    /* Connect to the server */
    if (connect(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) <0) {
        perror("connect()");
        exit(0);
    }

    char buf[MAX_LEN];

    FILE *fp = fdopen(sockfd, "r");

    do {
        fgets(buf, MAX_LEN-1, fp);
        printf("%s", buf);
    } while (!strncmp(buf, "220-", 4));

    close(sockfd);

    return 0;
}