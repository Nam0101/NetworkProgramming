#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <pthread.h>
#define MAX_BUFFER_SIZE 1024

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

struct sockaddr_in create_server_addr(int PORT)
{
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    return server_addr;
}

void bind_socket(int sockfd, struct sockaddr_in server_addr)
{
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error binding");
        printf("Error code: %d\n", 1);
        exit(1);
    }
}

char *str2md5(const char *str, int length)
{
    int n;
    MD5_CTX c;
    unsigned char digest[16];
    char *out = (char *)malloc(33);

    MD5_Init(&c);

    while (length > 0)
    {
        if (length > 512)
        {
            MD5_Update(&c, str, 512);
        }
        else
        {
            MD5_Update(&c, str, length);
        }
        length -= 512;
        str += 512;
    }

    MD5_Final(digest, &c);

    for (n = 0; n < 16; ++n)
    {
        snprintf(&(out[n * 2]), 16 * 2, "%02x", (unsigned int)digest[n]);
    }

    return out;
}


char* digitInMD5(const char*str){
    //return string contains only digit in MD5 of str
    char* digit = (char*)malloc(33);
    int j = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] >= '0' && str[i] <= '9')
        {
            digit[j] = str[i];
            j++;
        }
    }
    digit[j] = '\0';
    return digit;
}

char* charInMD5(const char* str){
    //return string contains only character in MD5 of str
    char* character = (char*)malloc(33);
    int j = 0;
    for (int i = 0; i < strlen(str); i++){
        if (str[i] >= 'a' && str[i] <= 'z')
        {
            character[j] = str[i];
            j++;
        }
    }
    character[j] = '\0';
    return character;
}

int isValidString(const char* str){
    // trả về true nếu str chỉ chứa kí tự từ a-z,A-Z và số từ 0-9, ngược lại trả về false
    for (int i = 0; i < strlen(str)-1; i++){
        if (!((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= '0' && str[i] <= '9') || (str[i] >= 'A' && str[i] <= 'Z'))){
            return 0;
        }
    }
    return 1;
}


int main(int argc, char *argv[])
{
    int PORT;
    int client1_set = 0;
    int client2_set = 0;
    if (argc < 2)
    {
        printf("Please enter port number\n");
        exit(1);
    }
    PORT = atoi(argv[1]);

    struct sockaddr_in server_addr = create_server_addr(PORT);
    struct sockaddr_in client_addr1, client_addr2, temp_addr;

    socklen_t client_addr_len = sizeof(client_addr1);
    socklen_t client_addr_len2 = sizeof(client_addr2);
    socklen_t temp_addr_len = sizeof(temp_addr);

    char buffer[MAX_BUFFER_SIZE];

    // Create a UDP socket
    int sockfd = create_socket();

    // Bind the socket to the specified port
    bind_socket(sockfd, server_addr);

    printf("UDP server is listening on port %d...\n", PORT);

    while (1)
    {
        ssize_t bytes_received;
        if (!client1_set)
        {
            printf("Waiting for client 1...\n");
            bytes_received = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&client_addr1, &client_addr_len);
            if (bytes_received >= 0)
            {
                client1_set = 1;
                buffer[bytes_received] = '\0';
                printf("Connected to client 1\n");
            }
        }

        if (!client2_set)
        {
            printf("Waiting for client 2...\n");
            bytes_received = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&client_addr2, &client_addr_len2);
            if (bytes_received >= 0)
            {
                client2_set = 1;
                printf("Connected to client 2\n");
            }
        }
        printf("Listening...\n");

        if (client1_set && client2_set)
        {
            memset(buffer, 0, sizeof(buffer));
            bytes_received = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&temp_addr, &temp_addr_len);
            if (bytes_received < 0)
            {
                perror("Error: ");
                close(sockfd);
                exit(1);
            }
            if(!isValidString(buffer)){
                printf("Invalid string\n");
                continue;
            }
            char *hash = str2md5(buffer, strlen(buffer));
            char *digit = digitInMD5(hash);
            char *character = charInMD5(hash);
            if (temp_addr.sin_port == client_addr1.sin_port)
            {
                printf("Received from client 1: %s", buffer);
                sendto(sockfd, digit, strlen(digit), 0, (const struct sockaddr *)&client_addr2, client_addr_len2);
                sendto(sockfd, character, strlen(character), 0, (const struct sockaddr *)&client_addr2, client_addr_len2);
            }
            else
            {
                printf("Received from client 2: %s", buffer);
                sendto(sockfd, digit, strlen(digit), 0, (const struct sockaddr *)&client_addr1, client_addr_len);
                sendto(sockfd, character, strlen(character), 0, (const struct sockaddr *)&client_addr1, client_addr_len);
            }
            free(hash);
            free(digit);
            free(character);
        }
        memset(buffer, 0, sizeof(buffer));
    }

    close(sockfd);

    return 0;
}
