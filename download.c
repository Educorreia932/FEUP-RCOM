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

    char* address = inet_ntoa(*((struct in_addr*) h->h_addr));

    // TCP
    int sockfd;
    struct sockaddr_in server_addr;

    // Server address handling

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

    if (connect(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(0);
    }

    /* Read from server */

    char buf[MAX_LEN];
    FILE* fp = fdopen(sockfd, "r");

    do {
        fgets(buf, MAX_LEN-1, fp);
        printf("%s", buf);
    } while (buf[3] == '-');

    if(buf[0] != '2'){  // not 220
        perror("Error in connection.\n");
        exit(1);
    }

    /* Login 
        user username\n
        pass password\n
    */

    // Send user
    if(write(sockfd, "user ", 5) < 0){
        perror("Failed to send user.\n");
        exit(1);
    }

    if(write(sockfd, fields.user, strlen(fields.user)) < 0){
        perror("Failed to send username.\n");
        exit(1);
    }

    if(write(sockfd, "\n", 1) < 0){
        perror("Failed to send newline.\n");
        exit(1);
    }

    // Read Response
    do {
        fgets(buf, MAX_LEN-1, fp);
        printf("%s", buf);
    } while (buf[3] == '-');

    // Check if it server sent "331 Please specify the password".
    // TODO: Check server other messages
    if(buf[0] != '3'){
        perror("Server didn't recognize user.\n");
        exit(1);
    } 

    // Send password
    if(write(sockfd, "pass ", 5) < 0){
        perror("Failed to send pass.\n");
        exit(1);
    }

    if(write(sockfd, fields.password, strlen(fields.password)) < 0){
        perror("Failed to send password.\n");
        exit(1);
    }

    if(write(sockfd, "\n", 1) < 0){
        perror("Failed to send newline.\n");
        exit(1);
    }

    // Read Response
    do {
        fgets(buf, MAX_LEN-1, fp);
        printf("%s", buf);
    } while (buf[3] == '-');

    // Check if it server sent "230 Login successful."
    // TODO: Check server other messages
    if (buf[0] != '2') {
        perror("Login was not successful.\n");
        exit(1);
    } 

    // Enter passive mode

    // send pasv
    if(write(sockfd, "pasv\n", 5) < 0){
        perror("Failed to send pasv.\n");
        exit(1);
    }

    // Read Response
    do {
        fgets(buf, MAX_LEN-1, fp);
        printf("%s", buf);
    } while (buf[3] == '-');

    /* Check if server sent "227 Entering Passive Mode (193,136,28,12,19,91)" */
    if (buf[0] != '2') {
        perror("Failed to enter passive mode.\n");
        exit(1);
    } 

    /* Get server port for file transfer */

    parseFilePort(buf);

    close(sockfd);

    return 0;
}