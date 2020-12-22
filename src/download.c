#include "utils.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(1);
    }

    // Parse & store arguments
    struct url url;
    if (parse_url(argv[1], &url) < 0) {
        puts("Aborting\n");
        exit(1);
    }

    struct hostent* h;
    if ((h = gethostbyname(url.host)) == NULL) {
        herror("gethostbyname");
        exit(1);
    }
    // Get IP
    char* address = inet_ntoa(*((struct in_addr*) h->h_addr));

    // Print info
    printf("User:       %s\n", url.user);
    puts("Password:   *****");
    printf("Host:       %s\n", url.host);
    printf("Filepath:        %s\n", url.filepath);
    printf("Host name:  %s\n", h->h_name);
    printf("IP Address: %s\n", address);
    printf("Port:       %d\n\n", SERVER_PORT);

    // Open socket
    int sockfd = create_socket(address, SERVER_PORT);
    FILE* fp = fdopen(sockfd, "r");

    // Welcome Message
    if (readFromSocket(fp) != '2') {
        printf("ERROR: Error in connection.\n");
        exit(1);
    }

    /* Login 
        user username\n
        pass password\n
    */

    // Send User
    char buf[MAX_LEN];
    sprintf(buf, "user %s\n", url.user);
    if (write(sockfd, buf, strlen(buf)) < 0) {
        printf("ERROR: Failed to send user.\n");
        exit(1);
    }

    // Read response
    char res = readFromSocket(fp);

    // Check if it server sent "331 Please specify the password".
    if (res == '3') {
        //Check if password is set
        if (!strcmp(url.password, "")) {
            if (strcmp(url.user, "anonymous")) {
                // Ask for user password
                printf("\nPlease input a password: ");
                char pass[MAX_LEN];
                fgets(pass, sizeof(pass), stdin);
                strcpy(url.password, pass);
            }
        }
        // Send password
        sprintf(buf, "pass %s\n", url.password);
        if (write(sockfd, buf, strlen(buf)) < 0) {
            printf("ERROR: Failed to send password.\n");
            exit(1);
        }
        // Read response
        if (readFromSocket(fp) != '2') {
            printf("ERROR: Login was not successful.\n");
            exit(1);
        }
    } else if (res != '2') {
        printf("ERROR: Failed to send user.\n");
        exit(1);
    }

    // Enter passive mode
    if (write(sockfd, "pasv\n", 5) < 0) {
        printf("ERROR: Failed to send pasv.\n");
        exit(1);
    }
    // Read response
    do {
        fgets(buf, MAX_LEN - 1, fp);
        printf("%s", buf);
    } while (buf[3] == '-');

    if (buf[0] != '2') {
        printf("ERROR: Failed to enter passive mode.\n");
        exit(1);
    }
    // Open socket
    int data_socket_fd = create_socket(address, get_port(buf));
    // Send retr command
    sprintf(buf, "retr %s\n", url.filepath);
    if (write(sockfd, buf, strlen(buf)) < 0) {
        printf("ERROR: Failed to send retr command.\n");
        exit(1);
    }
    // Read response
    do {
        fgets(buf, MAX_LEN - 1, fp);
        printf("%s", buf);
    } while (buf[3] == '-');

    if (buf[0] != '2' && buf[0] != '1') {
        printf("ERROR: Failed to retrieve file.\n");
        exit(1);
    }

    download_file(get_file_size(buf), data_socket_fd, url.filepath);
    close(data_socket_fd);
    close(sockfd);

    return 0;
}