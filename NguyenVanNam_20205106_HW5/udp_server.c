#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include "node.h"
#include <pthread.h>
#define MAX_BUFFER_SIZE 100000
#define ACTIVE 1
#define BLOCKED 0
#define logedIn 1
#define notLogedIn 0
const char *dataFileName = "account.txt";
int isLogin = 0;
// define a list to store account
list_t *list;
// define a node to store loged in account
node_t *logedInAccount;
// server address
struct sockaddr_in server_addr, client_addr;
socklen_t client_addr_len;
int sockfd;

// create a UDP socket
int create_socket()
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        exit(1);
    }
    return sockfd;
}

// create server address
struct sockaddr_in create_server_addr(int PORT)
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
        printf("Error code: %d\n", 1);
        exit(1);
    }
}

// read file and store data to message
void receive_message(char *message)
{
    char buffer[MAX_BUFFER_SIZE];
    int bytes_received = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
    if (bytes_received < 0)
    {
        perror("Error: ");
        close(sockfd);
        exit(1);
    }
    buffer[bytes_received - 1] = '\0';
    strcpy(message, buffer);
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

// return node_t that has userName
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
// case if can found an account
void foundedAccount(node_t *account)
{
    if (account->status == ACTIVE)
    {
        isLogin = 1;
        char *message = "OK";
        logedInAccount = account;
        sendto(sockfd, (const char *)message, strlen(message), 0, (const struct sockaddr *)&client_addr, client_addr_len);
        return;
    }
    char *message = "account not ready";
    sendto(sockfd, (const char *)message, strlen(message), 0, (const struct sockaddr *)&client_addr, client_addr_len);
}
// case if password is wrong
void passwordWrong(char *userName)
{
    char *message = "not OK";
    int isFound = 0;
    sendto(sockfd, (const char *)message, strlen(message), 0, (const struct sockaddr *)&client_addr, client_addr_len);

    node_t *account = findByUserName(list, userName);
    int numTry = 1;
    // try 3 times to login
    while (numTry < 3)
    {
        char *buffer = (char *)malloc(MAX_BUFFER_SIZE);
        receive_message(buffer);
        node_t *account = findInList(list, userName, buffer, &isFound);
        if (isFound)
        {
            if (account->status == ACTIVE)
            {
                isLogin = 1;
                char *message = "OK";
                logedInAccount = account;
                sendto(sockfd, (const char *)message, strlen(message), 0, (const struct sockaddr *)&client_addr, client_addr_len);
                return;
            }
            char *message = "account not ready";
            sendto(sockfd, (const char *)message, strlen(message), 0, (const struct sockaddr *)&client_addr, client_addr_len);
        }
        else
        {
            char *message = "not OK";
            sendto(sockfd, (const char *)message, strlen(message), 0, (const struct sockaddr *)&client_addr, client_addr_len);
            numTry++;
        }
    }
    // if login fail after 3 times, server will send or "block account" to client
    if (numTry == 3)
    {
        char *message = "block account";
        account = findByUserName(list, userName);
        sendto(sockfd, (const char *)message, strlen(message), 0, (const struct sockaddr *)&client_addr, client_addr_len);
        if (account != NULL)
        {
            account->status = BLOCKED;
            writeFile(list, dataFileName);
        }
    }
    return;
}
// if not login, then handle it
void notLoginHandle(list_t *list)
{
    char *buffer = (char *)malloc(MAX_BUFFER_SIZE);
    receive_message(buffer);
    printf("User name: %s\n", buffer);
    char *userName = strdup(buffer);
    receive_message(buffer);
    printf("Password: %s\n", buffer);
    char *password = strdup(buffer);
    int isFound = 0;
    node_t *account = findInList(list, userName, password, &isFound);
    if (isFound)
    {
        // if account is found, then check status of account
        foundedAccount(account);
    }
    else
    {
        // if account is not found, then check password
        passwordWrong(userName);
    }
}

int isValidPassword(char *password)
{
    // kiểm tra nếu password chứa ký tự khác chữ và số
    int i = 0;
    while (i < strlen(password))
    {
        if ((password[i] < '0' || password[i] > '9') && (password[i] < 'a' || password[i] > 'z') && (password[i] < 'A' || password[i] > 'Z'))
        {
            return 0;
        }
        i++;
    }
    return 1;
}
// hash password by sha256 and return hashed password
char *SHA256Hashing(char *password)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);
    char *output = (char *)malloc(65);
    int i = 0;
    for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\0';
    return output;
}
// return digit in string
char *digitInString(char *str)
{
    int i = 0;
    char *digit = (char *)malloc(MAX_BUFFER_SIZE);
    int j = 0;
    while (i < strlen(str))
    {
        if (str[i] >= '0' && str[i] <= '9')
        {
            digit[j] = str[i];
            j++;
        }
        i++;
    }
    digit[j] = '\0';
    return digit;
}
// return character in string
char *charInString(char *str)
{
    int i = 0;
    char *character = (char *)malloc(MAX_BUFFER_SIZE);
    int j = 0;
    while (i < strlen(str))
    {
        if ((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z'))
        {
            character[j] = str[i];
            j++;
        }
        i++;
    }
    character[j] = '\0';
    return character;
}
// if login, then handle it
void logedInHandle(list_t *list)
{
    char *buffer = (char *)malloc(MAX_BUFFER_SIZE);
    receive_message(buffer);
    // if client send "bye", then server will send "Goodbye <username>" to client
    if (strcmp(buffer, "bye") == 0)
    {
        isLogin = 0;
        char *message = "Goodbye ";
        char *send_message = (char *)malloc(MAX_BUFFER_SIZE);
        strcat(send_message, message);
        strcat(send_message, logedInAccount->userName);
        printf("%s\n", send_message);
        sendto(sockfd, (const char *)send_message, strlen(send_message), 0, (const struct sockaddr *)&client_addr, client_addr_len);
        return;
    }
    // receive new password from client, then hash it and store it to account and send digit and character to client
    if (isValidPassword(buffer))
    {
        char *hashedPassword = SHA256Hashing(buffer);
        char *digit = digitInString(hashedPassword);
        char *character = charInString(hashedPassword);
        node_t *account = findByUserName(list, logedInAccount->userName);
        strcpy(account->password, buffer);
        writeFile(list, dataFileName);
        sendto(sockfd, (const char *)digit, strlen(digit), 0, (const struct sockaddr *)&client_addr, client_addr_len);
        sendto(sockfd, (const char *)character, strlen(character), 0, (const struct sockaddr *)&client_addr, client_addr_len);
        return;
    }
    char *message = "Invalid password";
    sendto(sockfd, (const char *)message, strlen(message), 0, (const struct sockaddr *)&client_addr, client_addr_len);
    return;
}

int main(int argc, char *argv[])
{
    int PORT;
    if (argc < 2)
    {
        printf("Please enter port number\n");
        exit(1);
    }
    PORT = atoi(argv[1]);
    server_addr = create_server_addr(PORT);
    client_addr;
    client_addr_len = sizeof(client_addr);
    list = createList();
    readFile(list, dataFileName);
    char buffer[MAX_BUFFER_SIZE];
    // Create a UDP socket
    sockfd = create_socket();

    // Bind the socket to the specified port
    bind_socket(sockfd, server_addr);

    printf("UDP server is listening on port %d...\n", PORT);
    memset(buffer, 0, sizeof(buffer));
    // server will run forever until client send "bye"
    while (1)
    {
        if (!isLogin)
        {
            notLoginHandle(list);
        }
        else
        {
            logedInHandle(list);
        }
    }
}