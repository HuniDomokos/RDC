#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 4096

typedef struct {
    int client_fd;
} client_args_t;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void *handle_client(void *arg) {
    client_args_t *args = (client_args_t *)arg;
    int client_fd = args->client_fd;
    free(args);

    char buffer[BUFFER_SIZE];
    while (1) {
        ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            if (n < 0) {
                perror("recv");
            }
            break;
        }

        buffer[n] = '\0';

        int num1 = 0;
        int num2 = 0;
        char op = '\0';
        int parsed = sscanf(buffer, "%d %c %d", &num1, &op, &num2);
        char response[BUFFER_SIZE];

        if (parsed != 3) {
            snprintf(response, sizeof(response), "Error: Invalid expression\n");
            send(client_fd, response, strlen(response), 0);
            continue;
        }

        double result = 0.0;
        if (op == '+') {
            result = (double)num1 + (double)num2;
        } else if (op == '-') {
            result = (double)num1 - (double)num2;
        } else if (op == '*') {
            result = (double)num1 * (double)num2;
        } else if (op == '/') {
            if (num2 == 0) {
                snprintf(response, sizeof(response), "Error: Division by zero\n");
                send(client_fd, response, strlen(response), 0);
                continue;
            }
            result = (double)num1 / (double)num2;
        } else {
            snprintf(response, sizeof(response), "Error: Invalid operation\n");
            send(client_fd, response, strlen(response), 0);
            continue;
        }

        if (result == (int)result) {
            snprintf(response, sizeof(response), "Result: %d\n", (int)result);
        } else {
            snprintf(response, sizeof(response), "Result: %.2f\n", result);
        }

        send(client_fd, response, strlen(response), 0);
    }

    close(client_fd);
    return NULL;
}

int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN);

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int sockfd, newsockfd, portno;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("Error opening socket");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("Error on binding");
    }

    if (listen(sockfd, 5) < 0) {
        error("Error on listening");
    }

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("accept");
            continue;
        }

        client_args_t *args = malloc(sizeof(*args));
        if (args == NULL) {
            perror("malloc");
            close(newsockfd);
            continue;
        }

        args->client_fd = newsockfd;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, args) != 0) {
            perror("pthread_create");
            free(args);
            close(newsockfd);
            continue;
        }

        pthread_detach(thread_id);
    }

    close(sockfd);
    return 0;
}
