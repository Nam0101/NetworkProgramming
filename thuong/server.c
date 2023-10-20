#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/sha.h>

#define BUFF_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s PortNumber\n", argv[0]);
        exit(1);
    }

    int server_sock, client_sock;
    char buff[BUFF_SIZE];
    int bytes_sent, bytes_received;
    struct sockaddr_in server_addr, client_addr, client_addr2, temp_addr;

    // Step 1: Construct a UDP socket
    if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("\nError: ");
        exit(0);
    }

    // Step 2: Bind address to socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero), 8);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("\nError: ");
        exit(0);
    }

    char *connectStringC2 = "C2";
    char *connectStringC1 = "C1";
    socklen_t client_addr_len = sizeof(struct sockaddr);

    bytes_received = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&temp_addr, &client_addr_len);
    if (bytes_received < 0)
    {
        perror("Error: ");
        close(server_sock);
        return 0;
    }
    buff[bytes_received] = '\0';
    if (strcmp(buff, connectStringC1) == 0)
    {
        client_addr = temp_addr;
    }
    else if (strcmp(buff, connectStringC2) == 0)
    {
        client_addr2 = temp_addr;
    }
    bytes_received = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&temp_addr, &client_addr_len);
    if (bytes_received < 0)
    {
        perror("Error: ");
        close(server_sock);
        return 0;
    }
    buff[bytes_received] = '\0';
    if (strcmp(buff, connectStringC1) == 0)
    {
        client_addr = temp_addr;
    }
    else if (strcmp(buff, connectStringC2) == 0)
    {
        client_addr2 = temp_addr;
    }
    bytes_received = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&temp_addr, &client_addr_len);
    if (bytes_received < 0)
    {
        perror("Error: ");
        close(server_sock);
        return 0;
    }
    buff[bytes_received] = '\0';
    printf("Recived from client 1: %s\n", buff);
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((unsigned char *)buff, strlen(buff), hash);
    char *hash_str = (char *)malloc(SHA_DIGEST_LENGTH * 2 + 1);
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
    {
        sprintf(&hash_str[i * 2], "%02x", hash[i]);
    }

    char alpha_str[BUFF_SIZE], digit_str[BUFF_SIZE];
    int alpha_index = 0, digit_index = 0;
    for (int i = 0; i < strlen(hash_str); i++)
    {
        if (isalpha(hash_str[i]))
        {
            alpha_str[alpha_index++] = hash_str[i];
        }
        else
        {
            digit_str[digit_index++] = hash_str[i];
        }
    }
    alpha_str[alpha_index] = '\0';
    digit_str[digit_index] = '\0';

    char result[BUFF_SIZE];
    strcpy(result, alpha_str);
    sendto(server_sock, result, strlen(result), 0, (struct sockaddr *)&client_addr2, sizeof(struct sockaddr));
    strcpy(result, digit_str);
    sendto(server_sock, result, strlen(result), 0, (struct sockaddr *)&client_addr2, sizeof(struct sockaddr));
    close(server_sock);
    return 0;
}
