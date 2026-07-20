#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>

#define BUFFER_SIZE 4096

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char * argv[]){

    int sockfd, portno, n;
    char buffer[BUFFER_SIZE];

    struct sockaddr_in serv_addr;
    struct hostent *server;

    if (argc != 3) {
        error("Usage: ./FILENAME <hostname> <port number>\n");
    }

    portno = atoi(argv[2]);
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("Error opening socket");
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        error("Error connecting");
    }

    while(1){
        bzero(buffer, BUFFER_SIZE);
        n = read(sockfd, buffer, BUFFER_SIZE - 1);
        if (n < 0) 
            error("Error reading from socket");
        printf("Server: %s", buffer);
        if (strncmp(buffer, "Result: ", 8) == 0) {
            break; 
        }

        bzero(buffer, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE - 1, stdin);
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0)
            error("Error writing to socket");

    }

    close(sockfd);
    return 0;
}