#include "utils.h"

int parse_fields(char* arguments, struct fields* fields) {
    // Get protocol name

    if (strncmp(arguments, "ftp://", 6)) {
        perror("Couldn't parse the protocol name\n");
        return -1;
    }

    arguments += 6; // skip ftp://

    // Get user
    char* token = strtok(arguments, ":");

    if (token == NULL) { // No user sent
        strcpy(fields->user, "anonymous");  // Assume anon
        strcpy(fields->user, "pass");
    }

    
    else{  
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
