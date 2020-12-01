#include "utils.h"

#define STYLE_BOLD         "\033[1m"
#define STYLE_NO_BOLD      "\033[22m"

int create_socket(char* ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    // Server address handling
    bzero((char*) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip); /* 32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(port); /* server TCP port must be network byte ordered */

    // Open an TCP socket 

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(0);
    }

    /* Connect to the server */
    if (connect(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(0);
    }

    return sockfd;
}

void parse_file_port(char* str, char** ip, int* port) {
    str += 27;  // Skip useless info

    *ip = (char*) malloc(MAX_LEN);
    
    char* token = strtok(str, ",");
    char dot = '.';

    for (int i=0; i <= 5; i++) {
        if (i <= 3) {
            strcat(*ip, token);

            if (i != 3)
                strncat(*ip, &dot, 1);
        }

        else if (i == 4)
            *port = atoi(token) * 256;

        else
            *port += atoi(token); 

        token = strtok(NULL, ",");
    }

    printf("IP   : %s\n", *ip);
    printf("Port : %d\n", *port);
}

void print_fields(struct fields fields){
    printf(STYLE_BOLD);
    puts("Arguments\n");
    printf(STYLE_NO_BOLD);
    printf("User     : %s\n", fields.user);
    printf("Password : %s\n", fields.password);
    printf("Host     : %s\n", fields.host);
    printf("URL      : %s\n", fields.url);
    puts("");
}

int parse_fields(char* arguments, struct fields* fields) {
    // Get protocol name

    if (strncmp(arguments, "ftp://", 6)) {
        perror("Couldn't parse the protocol name\n");
        return -1;
    }

    arguments += 6; // Skip ftp://

    // Get user
    char* token = strtok(arguments, ":");

    if (token == NULL) { // No user sent
        strcpy(fields->user, "anonymous");  // Assume anon
        strcpy(fields->user, "pass");
    }
    
    else {  
        strcpy(fields->user, token);

        // Get password
        token = strtok(NULL, "@");

        if (token == NULL) {
            perror("Couldn't parse the password\n");
            return -1;
        }
        
        strcpy(fields->password, token);
    }

    // Get host
    token = strtok(NULL, "/");

    if (token == NULL) {
        perror("Couldn't parse the host\n");
        return -1;
    }

    strcpy(fields->host, token);

    // Get URL path

    token = strtok(NULL, "");

    if (token == NULL) {
        perror("Couldn't parse the URL path\n");
        return -1;
    }

    strcpy(fields->url, token);

    return 0;
}
