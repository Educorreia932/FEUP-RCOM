#include "utils.h"

#define STYLE_BOLD         "\033[1m"
#define STYLE_NO_BOLD      "\033[22m"

void parseFilePort(char * str){
    str += 27;  // Skip useless info

    char ip[MAX_LEN] = "";
    int port;
    
    char* token = strtok(str, ",");
    char dot = '.';

    for(int i=0; i< 5; i++) {
        if (i <= 3) {
            strcat(ip, token);

            if (i != 3)
                strncat(ip, &dot, 1);
        }

        else if (i == 4)
            port = atoi(token) * 256;

        else
            port += atoi(token); 

        token = strtok(NULL, ",");
    }

    printf("IP   : %s\n", ip);
    printf("Port : %d\n", port);
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
