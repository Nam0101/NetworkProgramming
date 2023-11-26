#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "node.h"
int PORT;
#define ACTIVE 1
#define BLOCKED 0
#define LOGIN 2
#define logedIn 1
#define notLogedIn 0
#define MAX_BUFFER_SIZE 4096
const char *dataFileName = "account.txt";
list_t *list;
// mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// create a TCP socket
int create_socket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        exit(1);
    }
    return sockfd;
}

// create server address
struct sockaddr_in create_server_addr()
{
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    return server_addr;
}

// bind socket to server address and port
void bind_socket(int sockfd, struct sockaddr_in server_addr)
{
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error binding");
        exit(1);
    }
    if (listen(sockfd, 5) < 0)
    {
        perror("Error listening");
        exit(1);
    }
}
void recvice_message(int sockfd, char *message)
{
    ssize_t num_bytes = recv(sockfd, message, sizeof(message), 0);
    if (num_bytes == 0)
    {
        printf("Client closed connection\n");
        close(sockfd);
        return;
    }
    else if (num_bytes == -1)
    {
        perror("Error reading from client");
        close(sockfd);
        return;
    }
    // Null-terminate the message to safely use it as a string
    message[num_bytes] = '\0';
}
// write data from list to file
void writeFile(list_t *list, const char *fileName)
{
    FILE *fp = fopen(fileName, "w");
    if (fp == NULL)
    {
        printf("Cannot open %s file!\n", fileName);
        exit(1);
    }
    node_t *cur = list->head;
    while (cur != NULL)
    {
        fprintf(fp, "%s %s %hd\n", cur->userName, cur->password, cur->status);
        cur = cur->next;
    }
    fclose(fp);
}

// return node_t that has userName and password
node_t *findInList(list_t *list, char *userName, char *password, int *isFound)
{
    node_t *cur = list->head;
    while (cur != NULL)
    {
        if (strcmp(cur->userName, userName) == 0 && strcmp(cur->password, password) == 0)
        {
            *isFound = 1;
            return cur;
        }
        cur = cur->next;
    }
    return cur;
}
node_t *findByUserName(list_t *list, char *userName)
{
    node_t *cur = list->head;
    while (cur != NULL)
    {
        if (strcmp(cur->userName, userName) == 0)
        {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

void passwordWrong(int client_sockfd, char *userName, node_t *logedInAccount)
{
    char *message = "not OK";
    int isFound = 0;
    send(client_sockfd, message, strlen(message), 0);

    node_t *account = findByUserName(list, userName);
    int numTry = 1;
    // try 3 times to login
    while (numTry < 3)
    {
        char *buffer = (char *)malloc(MAX_BUFFER_SIZE);
        recv(client_sockfd, buffer, MAX_BUFFER_SIZE, 0);
        node_t *account = findInList(list, userName, buffer, &isFound);
        if (isFound)
        {
            if (account->status == ACTIVE)
            {
                char *message = "OK";
                logedInAccount = account;
                send(client_sockfd, message, strlen(message), 0);
                return;
            }
            char *message = "account not ready";
            send(client_sockfd, message, strlen(message), 0);
        }
        else
        {
            char *message = "not OK";
            send(client_sockfd, message, strlen(message), 0);
            numTry++;
        }
    }
    // if login fail after 3 times, server will send or "block account" to client
    if (numTry == 3)
    {
        char *message = "block account";
        account = findByUserName(list, userName);
        send(client_sockfd, message, strlen(message), 0);
        if (account != NULL)
        {
            account->status = BLOCKED;
            pthread_mutex_lock(&mutex);
            writeFile(list, dataFileName);
            readFile(list, dataFileName);
            pthread_mutex_unlock(&mutex);
        }
    }
    printf("Client closed connection\n");
    return;
}
// handle client
void *handle_client(void *arg)
{
    // handle client's requests here
    int client_sockfd = *(int *)arg;
    char *message = (char *)malloc(MAX_BUFFER_SIZE);
    printf("Client connected\n");
    char *userName = (char *)malloc(100);
    char *password = (char *)malloc(100);
    recvice_message(client_sockfd, userName);
    recvice_message(client_sockfd, password);
    printf("Username: %s\n", userName);
    printf("Password: %s\n", password);
    int isFound = 0;
    // create new list to read data from file
    list_t *new_list = createList();
    // lock
    if (pthread_mutex_trylock(&mutex) == 0)
    {
        readFile(new_list, dataFileName);
        pthread_mutex_unlock(&mutex);
    }
    else
    {
        printf("Cannot lock mutex\n");
    }

    // unlock
    node_t *account = findInList(new_list, userName, password, &isFound);
    if (isFound)
    {
        // case of status
        if (account->status == BLOCKED)
        {
            send(client_sockfd, "block account", strlen("block account"), 0);
            return NULL;
        }
        else if (account->status == LOGIN)
        {
            char *message = "AALG";
            send(client_sockfd, message, strlen(message), 0);
            return NULL;
        }
        send(client_sockfd, "OK", strlen("OK"), 0);
        // account in list status is login
        account->status = LOGIN;
        // lock
        pthread_mutex_lock(&mutex);
        writeFile(new_list, dataFileName);
        // unlock
        pthread_mutex_unlock(&mutex);
        recvice_message(client_sockfd, message);
        account->status = ACTIVE;
        pthread_mutex_lock(&mutex);
        writeFile(new_list, dataFileName);
        pthread_mutex_unlock(&mutex);
        printf("Client closed connection\n");
        close(client_sockfd);
        return NULL;
    }
    else
    {
        // handle when wrong password
        passwordWrong(client_sockfd, userName, account);
    }
    free(message);
    free(userName);
    free(password);
    // destroy list
    free(new_list);
    free(account);
    // destroy thread
    pthread_detach(pthread_self());
    return NULL;
}
int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    int sockfd;
    if (argc < 2)
    {
        printf("Please enter port number\n");
        printf("Usage: ./server <port>\n");
        exit(1);
    }
    PORT = atoi(argv[1]);
    // Create a TCP socket
    sockfd = create_socket();

    // Create server address
    server_addr = create_server_addr();

    // Bind the socket to the specified port
    bind_socket(sockfd, server_addr);

    printf("TCP server is listening on port %d...\n", PORT);
    list = createList();
    readFile(list, dataFileName);
    node_t *cur = list->head;
    while (cur != NULL)
    {
        printf("%s %s %hd\n", cur->userName, cur->password, cur->status);
        cur = cur->next;
    }
    while (1)
    {
        socklen_t client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sockfd < 0)
        {
            perror("Error accepting connection");
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, &client_sockfd);
    }

    return 0;
}