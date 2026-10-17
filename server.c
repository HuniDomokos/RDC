#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#define BUFFER 1024

int main(int argc, char const* argv[]){
    
    int sockfd = socket(AF_INET,SOCK_DGRAM,0);
    int rec;

    struct sockaddr_in address, client;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(atoi(argv[1]));

    if(bind(sockfd, &address, sizeof(address)) < 0) {
        perror("bind");
        return 1;
    }

    char buf[BUFFER];
    rec = recvfrom(sockfd, buf, BUFFER, 0, &client, &addrlen);
    buf[rec] = '\0';

    printf("Received message: Length: %ld %s\n", strlen(buf), buf);


    strcpy(buf, "Hello from server");
    sendto(sockfd, buf, strlen(buf), 0, &client, addrlen);
    return 0;
}