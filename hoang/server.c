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
    int PORT;
    if(argc != 2){
        printf("Usage: %s PortNumber\n", argv[0]);
        exit(1);
    }
    else{
        PORT = atoi(argv[1]);
    }
    int server_sock; /* file descriptors */
    char buff[BUFF_SIZE];
    int bytes_sent, bytes_received;
    struct sockaddr_in server;                       /* server's address information */
    struct sockaddr_in client1, client2, tempclient; /* client's address information */
    socklen_t sin_size;

    // Step 1: Construct a UDP socket
    if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    { /* calls socket() */
        perror("\nError 1 : ");
        exit(0);
    }

    // Step 2: Bind address to socket
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);       /* Remember htons() from "Conversions" section? =) */
    server.sin_addr.s_addr = INADDR_ANY; /* INADDR_ANY puts your IP address automatically */
    bzero(&(server.sin_zero), 8);        /* zero the rest of the structure */

    if (bind(server_sock, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    { /* calls bind() */
        perror("\nError2: ");
        exit(0);
    }
    bytes_received = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&tempclient, &sin_size);
    if (bytes_received < 0)
    {
        perror("Error3: ");
        close(server_sock);
        return 0;
    }
    buff[bytes_received] = '\0';
    if (strcmp(buff, "Client1") == 0)
    {
        client1 = tempclient;
    }
    else if (strcmp(buff, "Client2") == 0)
    {
        client2 = tempclient;
    }
    bytes_received = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&tempclient, &sin_size);
    if (bytes_received < 0)
    {
        perror("Error4: ");
        close(server_sock);
        return 0;
    }
    buff[bytes_received] = '\0';
    if (strcmp(buff, "Client1") == 0)
    {
        client1 = tempclient;
    }
    else if (strcmp(buff, "Client2") == 0)
    {
        client2 = tempclient;
    }
    unsigned char hash[SHA_DIGEST_LENGTH];
    while (1)
    {
        sin_size = sizeof(struct sockaddr_in);

        bytes_received = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&client1, &sin_size);

        char alpha_str[BUFF_SIZE], digit_str[BUFF_SIZE];
        if (bytes_received < 0)
        {
            perror("\nError: ");
            close(server_sock);
            return 0;
        }

        buff[bytes_received] = '\0';
        printf("[%s:%d]: %s", inet_ntoa(client1.sin_addr), ntohs(client1.sin_port), buff);
        SHA1((unsigned char *)buff, strlen(buff), hash);
        char *hash_str = (char *)malloc(SHA_DIGEST_LENGTH * 2 + 1);
        for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
        {
            sprintf(&hash_str[i * 2], "%02x", hash[i]);
        }

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
        printf("\nAlpha string: %s\nDigit string: %s\n", alpha_str, digit_str);

        bytes_sent = sendto(server_sock, alpha_str, strlen(alpha_str), 0, (struct sockaddr *)&client2, sin_size); /* send to the client welcome message */
        if (bytes_sent < 0)
            perror("\nError: ");

        bytes_sent = sendto(server_sock, digit_str, strlen(digit_str), 0, (struct sockaddr *)&client2, sin_size); /* send to the client welcome message */
        if (bytes_sent < 0)
            perror("\nError s: ");
    }

    close(server_sock);
    return 0;
}
