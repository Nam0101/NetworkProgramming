#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <pthread.h>

#define MAX_BUFFER_SIZE 100000
int isLogin = 0;
// socket file descriptor
int sockfd;
// server address
struct sockaddr_in server_addr;

// receive message from server
void receive_message(char *message)
{
    char buffer[MAX_BUFFER_SIZE];
    socklen_t server_addr_len = sizeof(server_addr);

    int bytes_received = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &server_addr_len);
    if (bytes_received < 0)
    {
        perror("Error: ");
        close(sockfd);
        exit(1);
    }
    buffer[bytes_received] = '\0';
    strcpy(message, buffer);
}

// login to server, send username and password to server
// if username and password is correct, server will send "OK" to client
// else server will send "not OK" to client
// try 3 times to login
void login()
{
    char message[MAX_BUFFER_SIZE];

    printf("Insert Username:\n");
    fgets(message, sizeof(message), stdin);

    sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    memset(message, 0, sizeof(message));
    printf("Insert Password:\n");
    fgets(message, sizeof(message), stdin);

    sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    receive_message(message);
    printf("%s\n", message);
    if (strcmp(message, "not OK") == 0)
    {
        int numTry = 1;
        // try 3 times to login
        while (numTry < 3)
        {
            memset(message, 0, sizeof(char)*MAX_BUFFER_SIZE);
            printf("Insert Password:\n");
            fgets(message, sizeof(message), stdin);
            sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));
            receive_message(message);
            printf("%s\n", message);
            // if login success, server will send "OK" to client 
            if (strcmp(message, "OK") == 0)
            {
                isLogin = 1;
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
            receive_message(message);
            printf("%s\n", message);
            return;
        }
    }
    // if login success, server will send "OK" to client
    else if(strcmp(message, "OK") == 0){
        isLogin = 1;
        return;
    }
}

int main(int argc, char *argv[])
{
    int PORT;
    char *SERVER_IP;
    char message[MAX_BUFFER_SIZE];
    if (argc < 3)
    {
        printf("Please enter port number and server IP\n");
        printf("Usage: ./client <server_ip> <port>\n");
        exit(1);
    }
    PORT = atoi(argv[2]);
    SERVER_IP = argv[1];

    // Create a UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
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

    login();
    // if login success, client can change password
    // receive message from server
    while(isLogin){
        printf("Insert new password:\n");
        fgets(message, sizeof(message), stdin);
        if(strlen(message) == 1 ){
            break;
        }
        sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));
        memset(message, 0, sizeof(message));
        receive_message(message);
        printf("%s\n",message);
        if (strcmp(message, "Invalid password") != 0)
        {
            memset(message, 0, sizeof(message));
            receive_message(message);
            printf("%s\n", message);
        }
    }
    close(sockfd);

    return 0;
}
