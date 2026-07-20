#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define BUFFER_SIZE 4096

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char * argv[]){
    signal(SIGCHLD, SIG_IGN); // Ignore child process termination signals to prevent zombie processes
    if (argc != 2) {
        error("Port number not provided. Program terminated.\n");
    }       

    int sockfd, newsockfd, portno, n;
    char buffer[BUFFER_SIZE];

    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("Error opening socket");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));

    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
        error("Error on binding");
    }

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);


    while(1){
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            error("Error on accept");
        }

        pid_t pid = fork();
        if (pid < 0) {
            error("Error on fork");
        }

        if (pid == 0){
            close(sockfd);
            while(1){
                n = write(newsockfd, "Enter first number: ", strlen("Enter first number: "));
                if (n < 0) 
                    error("Error writing to socket");
                bzero(buffer, BUFFER_SIZE);
                n = read(newsockfd, buffer, BUFFER_SIZE - 1);
                if (n < 0) 
                    error("Error reading from socket");
                int num1 = atoi(buffer);
                fprintf(stdout, "Received first number: %d\n", num1);

                n = write(newsockfd, "Enter second number: ", strlen("Enter second number: "));
                if (n < 0)
                    error("Error writing to socket");
                bzero(buffer, BUFFER_SIZE);
                n = read(newsockfd, buffer, BUFFER_SIZE - 1);
                if (n < 0)
                    error("Error reading from socket");
                int num2 = atoi(buffer);
                fprintf(stdout, "Received second number: %d\n", num2);

                n = write(newsockfd, "Enter operation (+, -, *, /): ", strlen("Enter operation (+, -, *, /): "));
                if (n < 0)
                    error("Error writing to socket");

                bzero(buffer, BUFFER_SIZE);
                n = read(newsockfd, buffer, BUFFER_SIZE - 1);
                if (n < 0)
                    error("Error reading from socket");
                fprintf(stdout, "Received operation: %c\n", buffer[0]);
                switch (buffer[0]) {
                    case '+':
                        sprintf(buffer, "Result: %d\n", num1 + num2);
                        break;
                    case '-':
                        sprintf(buffer, "Result: %d\n", num1 - num2);
                        break;
                    case '*':
                        sprintf(buffer, "Result: %d\n", num1 * num2);
                        break;
                    case '/':
                        if (num2 == 0) {
                            sprintf(buffer, "Error: Division by zero\n");
                        } else {
                            sprintf(buffer, "Result: %d\n", num1 / num2);
                        }
                        break;
                    default:
                        sprintf(buffer, "Error: Invalid operation\n");
                }
                n = write(newsockfd, buffer, strlen(buffer));
                if (n < 0)
                    error("Error writing to socket");
            }
            close(newsockfd);
            exit(0);
        } else {
            close(newsockfd);
        }
    }
    return 0;
}