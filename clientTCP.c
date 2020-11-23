#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_PORT 6000
#define SERVER_ADDR "192.168.28.96"

int main(int argc, char** argv) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buf[] = "Mensagem de teste na travessia da pilha TCP/IP\n";
    int bytes;

    /* Server address handling */
    bzero((char*) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR); /* 32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(SERVER_PORT); /* server TCP port must be network byte ordered */

    /* Open an TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(0);
    }

    /* Connect to the server */
    if (connect(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) <
        0) {
        perror("connect()");
        exit(0);
    }

    /* Send a string to the server */
    bytes = write(sockfd, buf, strlen(buf));
    printf("Bytes escritos %d\n", bytes);

    close(sockfd);
    exit(0);
}
