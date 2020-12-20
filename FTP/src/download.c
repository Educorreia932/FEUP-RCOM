#include "utils.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(1);
    }

    // Parse & store arguments
    struct fields fields;
    if (parse_fields(argv[1], &fields) < 0) {
        puts("Aborting\n");
        exit(1);
    }

    struct hostent* h;
    if ((h = gethostbyname(fields.host)) == NULL) {
        herror("gethostbyname");
        exit(1);
    }
    char* address = inet_ntoa(*((struct in_addr*) h->h_addr));

    // Print info
    printf("User:       %s\n", fields.user);
    puts("Password:   *****");
    printf("Host:       %s\n", fields.host);
    printf("URL:        %s\n", fields.url);
    printf("Host name:  %s\n", h->h_name);
    printf("IP Address: %s\n", address);
    printf("Port:       %d\n\n", SERVER_PORT);

    // Open socket
    int sockfd = create_socket(address, SERVER_PORT);
    FILE* fp = fdopen(sockfd, "r");

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
    sprintf(buf, "user %s\n", fields.user);
    if (write(sockfd, buf, strlen(buf)) < 0) {
        printf("ERROR: Failed to send user.\n");
        exit(1);
    }
    char res = readFromSocket(fp);

    // Check if it server sent "331 Please specify the password".
    if (res == '3') {
        //Check if password is set
        if (!strcmp(fields.password, "")) {
            if (strcmp(fields.user, "anonymous")) {
                printf("\nPlease input a password: ");
                char pass[MAX_LEN];
                fgets(pass, sizeof(pass), stdin);
                strcpy(fields.password, pass);
            }
        }
        // Send password
        sprintf(buf, "pass %s\n", fields.password);
        if (write(sockfd, buf, strlen(buf)) < 0) {
            printf("ERROR: Failed to send password.\n");
            exit(1);
        }

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

    // Read Response
    do {
        fgets(buf, MAX_LEN - 1, fp);
        printf("%s", buf);
    } while (buf[3] == '-');

    if (buf[0] != '2') {
        printf("ERROR: Failed to enter passive mode.\n");
        exit(1);
    }

    int port = get_port(buf);
    int data_socket_fd = create_socket(address, port);

    sprintf(buf, "retr %s\n", fields.url);
    if (write(sockfd, buf, strlen(buf)) < 0) {
        printf("ERROR: Failed to send retr command.\n");
        exit(1);
    }

    do {
        fgets(buf, MAX_LEN - 1, fp);
        printf("%s", buf);
    } while (buf[3] == '-');

    if (buf[0] != '2' && buf[0] != '1') {
        printf("ERROR: Failed to retrieve file.\n");
        close(data_socket_fd);
        close(sockfd);
        exit(1);
    }

    int file_size = get_file_size(buf);
    download_file(file_size, data_socket_fd, fields.url);

    close(data_socket_fd);
    close(sockfd);

    return 0;
}