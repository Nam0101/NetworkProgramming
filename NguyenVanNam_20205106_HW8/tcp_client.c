#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <pthread.h>
#define AESKEY "20205106"
#define MAX_BUFFER_SIZE 4096
int isLogin = 0;
// socket file descriptor
int sockfd;
// server address
struct sockaddr_in server_addr;

void onSucessCase()
{
    char message[MAX_BUFFER_SIZE];
    printf("Type anything to logout: \n");
    fgets(message, sizeof(message), stdin);
    message[strlen(message) - 1] = '\0';
    send(sockfd, message, strlen(message), 0);
    isLogin = 0;
}

// login to server, send username and password to server
// if username and password is correct, server will send "OK" to client
// else server will send "not OK" to client
// try 3 times to login
void login()
{
    char *message = (char *)malloc(MAX_BUFFER_SIZE);

    printf("Insert Username:\n");
    fgets(message, sizeof(message), stdin);
    // bo \n
    message[strlen(message) - 1] = '\0';

    int bytesent = send(sockfd, message, strlen(message), 0);
    if (bytesent < 0)
    {
        perror("Error: ");
        close(sockfd);
        exit(1);
    }
    memset(message, 0, strlen(message));
    printf("Insert Password:\n");
    fgets(message, sizeof(message), stdin);
    // bo \n
    message[strlen(message) - 1] = '\0';
    send(sockfd, message, strlen(message), 0);
    memset(message, 0, strlen(message));
    recv(sockfd, message, sizeof(message), 0);
    if (strcmp(message, "not OK") == 0)
    {
        printf("%s\n", message);
        int numTry = 1;
        // try 3 times to login
        while (numTry < 3)
        {
            memset(message, 0, sizeof(char) * MAX_BUFFER_SIZE);
            printf("Insert Password:\n");
            fgets(message, sizeof(message), stdin);
            send(sockfd, message, strlen(message), 0);
            memset(message, 0, sizeof(char) * MAX_BUFFER_SIZE);
            recv(sockfd, message, sizeof(message), 0);
            printf("%s\n", message);
            // if login success, server will send "OK" to client
            if (strcmp(message, "OK") == 0)
            {
                onSucessCase();
                return;
            }
            if (strcmp(message, "account not ready") == 0 || strcmp(message, "block account") == 0)
            {
                return;
            }
            numTry++;
        }
        // if login fail after 3 times, server will send "account not ready" or "block account" to client
        if (numTry == 3)
        {
            memset(message, 0, sizeof(char) * MAX_BUFFER_SIZE);
            recv(sockfd, message, sizeof(message), 0);
            printf("%s\n", message);
            return;
        }
    }
    // if login success, server will send "OK" to client
    else if (strcmp(message, "OK") == 0)
    {
        printf("%s\n", message);
        onSucessCase();
        return;
    }
    else
    {
        printf("Account already logged in\n");
        return;
    }
    free(message);
}

int main(int argc, char *argv[])
{
    int PORT;
    char *SERVER_IP;
    char *message = (char *)malloc(MAX_BUFFER_SIZE);
    if (argc < 3)
    {
        printf("Please enter port number and server IP\n");
        printf("Usage: ./client <server_ip> <port>\n");
        exit(1);
    }
    PORT = atoi(argv[2]);
    SERVER_IP = argv[1];

    // Create a UDP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        close(sockfd);
        exit(1);
    }
    // get connect to server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error connecting");
        close(sockfd);
        exit(1);
    }
    login();

    close(sockfd);
    free(message);

    return 0;
}
