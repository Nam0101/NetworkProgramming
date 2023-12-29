#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include "message.h"
#include <pthread.h>
#define AESKEY "20205106"
#define MAX_BUFFER_SIZE 1024
int isLogin = 0;
// socket file descriptor
int sockfd;
#define ACTIVE 1
#define BLOCKED 0
#define LOGEDIN 2
#define WRONG_PASSWORD 3
char userName[30];
// server address
struct sockaddr_in server_addr;
void sendChat()
{
    printf("Insert message:\n");
    char message[256];
    fgets(message, sizeof(message), stdin);
    // bo \n
    message[strlen(message) - 1] = '\0';
    Message *msg = (Message *)malloc(sizeof(Message));
    msg->type = CHAT;
    strcpy(msg->data.chat.username, userName);
    strcpy(msg->data.chat.message, message);
    send(sockfd, msg, sizeof(Message), 0);
    free(msg);
}
void sendFile()
{
    printf("Enter file name:\n");
    char fileName[30];
    fgets(fileName, sizeof(fileName), stdin);
    // bo \n
    fileName[strlen(fileName) - 1] = '\0';

    FILE *f = fopen(fileName, "rb");
    if (f == NULL)
    {
        printf("File not found\n");
        return;
    }

    // Get file size
    fseek(f, 0, SEEK_END);
    int fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    Message *msg = (Message *)malloc(sizeof(Message));
    msg->type = UPLOADFILE;
    strcpy(msg->data.uploadfile.username, userName);
    strcpy(msg->data.uploadfile.filename, fileName);
    msg->data.uploadfile.size = fileSize;

    // Send the initial message with file information
    send(sockfd, msg, sizeof(Message), 0);

    // Send the file data in chunks
    int bytesRead;
    char *buffer = (char *)malloc(MAX_BUFFER_SIZE);
    while ((bytesRead = fread(buffer, 1, MAX_BUFFER_SIZE, f)) > 0)
    {
        send(sockfd, buffer, bytesRead, 0);
    }
    // Close the file and free allocated memory
    fclose(f);
    free(msg);
}
void onSucessCase()
{
    Message *msg = (Message *)malloc(sizeof(Message));

    while (1)
    {
        printf("MENU\n");
        printf("1. Send Message\n");
        printf("2. Send Image\n");
        printf("3. Logout\n");
        int choice;
        scanf("%d", &choice);
        getchar();
        switch (choice)
        {
        case 1:
            sendChat();
            break;
        case 2:
            sendFile();
            break;
        case 3:
            msg->type = LOGOUT;
            strcpy(msg->data.logout.username, userName);
            send(sockfd, msg, sizeof(Message), 0);
            free(msg);
            return;
            break;
        default:
            break;
        }
    }
}

void wrong_password()
{
    Message *msg = (Message *)malloc(sizeof(Message));
    int numTry = 1;
    // try 3 times to login
    while (numTry < 3)
    {
        memset(msg, 0, sizeof(Message));
        msg->type = LOGIN;
        printf("Insert Password:\n");
        fgets(msg->data.login.password, sizeof(msg->data.login.password), stdin);
        // bo \n
        msg->data.login.password[strlen(msg->data.login.password) - 1] = '\0';
        send(sockfd, msg, sizeof(Message), 0);
        memset(msg, 0, sizeof(Message));
        recv(sockfd, msg, sizeof(Message), 0);
        // if login success, server will send "OK" to client
        if (msg->type == RESPONSE)
        {
            if (msg->data.response.status == ACTIVE)
            {
                printf("Login success\n");
                onSucessCase();
                return;
            }
            else if (msg->data.response.status == BLOCKED)
            {
                printf("Account has been blocked\n");
                return;
            }
            else if (msg->data.response.status == LOGEDIN)
            {
                printf("Account already logged in\n");
                return;
            }
            else if (msg->data.response.status == WRONG_PASSWORD)
            {
                printf("Wrong password\n");
            }
        }
        numTry++;
    }
    printf("Account has been blocked\n");
}
// login to server, send username and password to server
// if username and password is correct, server will send "OK" to client
// else server will send "not OK" to client
// try 3 times to login
void login()
{
    Message *msg = (Message *)malloc(sizeof(Message));
    msg->type = LOGIN;
    printf("Insert Username:\n");
    fgets(msg->data.login.username, sizeof(msg->data.login.username), stdin);
    // bo \n
    msg->data.login.username[strlen(msg->data.login.username) - 1] = '\0';
    strcpy(userName, msg->data.login.username);
    printf("Insert Password:\n");
    fgets(msg->data.login.password, sizeof(msg->data.login.password), stdin);
    // bo \n
    msg->data.login.password[strlen(msg->data.login.password) - 1] = '\0';
    send(sockfd, msg, sizeof(Message), 0);
    // receive message from server
    recv(sockfd, msg, sizeof(Message), 0);
    switch (msg->type)
    {
    case RESPONSE:
        if (msg->data.response.status == ACTIVE)
        {
            printf("Login success\n");
            onSucessCase();
            return;
        }
        else if (msg->data.response.status == BLOCKED)
        {
            printf("Account has been blocked\n");
            return;
        }
        else if (msg->data.response.status == LOGEDIN)
        {
            printf("Account already logged in\n");
            return;
        }
        else if (msg->data.response.status == WRONG_PASSWORD)
        {
            printf("Wrong password\n");
            wrong_password();
            return;
        }
        break;

    default:
        break;
    }
    // if (strcmp(message, "not OK") == 0)
    // {
    //     printf("%s\n", message);
    //     int numTry = 1;
    //     // try 3 times to login
    //     while (numTry < 3)
    //     {
    //         memset(message, 0, sizeof(char) * MAX_BUFFER_SIZE);
    //         printf("Insert Password:\n");
    //         fgets(message, sizeof(message), stdin);
    //         send(sockfd, message, strlen(message), 0);
    //         memset(message, 0, sizeof(char) * MAX_BUFFER_SIZE);
    //         recv(sockfd, message, sizeof(message), 0);
    //         printf("%s\n", message);
    //         // if login success, server will send "OK" to client
    //         if (strcmp(message, "OK") == 0)
    //         {
    //             onSucessCase();
    //             return;
    //         }
    //         if (strcmp(message, "account not ready") == 0 || strcmp(message, "BA") == 0)
    //         {
    //             return;
    //         }
    //         numTry++;
    //     }
    //     // if login fail after 3 times, server will send "account not ready" or "block account" to client
    //     if (numTry == 3)
    //     {
    //         memset(message, 0, sizeof(char) * MAX_BUFFER_SIZE);
    //         recv(sockfd, message, sizeof(message), 0);
    //         printf("%s\n", "account has been blocked");
    //         return;
    //     }
    // }
    // // if login success, server will send "OK" to client
    // else if (strcmp(message, "OK") == 0)
    // {
    //     printf("%s\n", message);
    //     onSucessCase();
    //     return;
    // }
    // else if (strcmp(message, "AALG") == 0)
    // {
    //     printf("Account already logged in\n");
    //     return;
    // }
    // else if (strcmp(message, "BA") == 0)
    // {
    //     printf("Account has been blocked\n");
    //     return;
    // }
    // free(message);
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
