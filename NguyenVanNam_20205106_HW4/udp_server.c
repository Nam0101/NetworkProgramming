#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <pthread.h>
#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    int sockfd;
    int PORT;
    int client1_set = 0;
    int client2_set = 0;
    if (argc < 2)
    {
        printf("Please enter port number\n");
        exit(1);
    }
    PORT = atoi(argv[1]);
    struct sockaddr_in server_addr, client_addr, client_addr2;
    socklen_t client_addr_len = sizeof(client_addr);
    socklen_t client_addr_len2 = sizeof(client_addr2);
    char buffer[MAX_BUFFER_SIZE];

    // Create a UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the specified port
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error binding");
        printf("Error code: %d\n", 1);
        exit(1);
    }

    printf("UDP server is listening on port %d...\n", PORT);
    while (1)
    {
        ssize_t bytes_received;
        if (!client1_set)
        {
            bytes_received = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
            if (bytes_received >= 0)
            {
                client1_set = 1;
                buffer[bytes_received] = '\0';
                printf("Received message from client 1: %s\n", buffer);
                printf("Client 1 address: %s\n", inet_ntoa(client_addr.sin_addr));
                printf("Client 1 port: %d\n", ntohs(client_addr.sin_port));
            }
        }

        if (!client2_set)
        {
            bytes_received = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&client_addr2, &client_addr_len2);
            if (bytes_received >= 0)
            {
                client2_set = 1;
                printf("Received message from client 2: %s\n", buffer);
                printf("Client 2 address: %s\n", inet_ntoa(client_addr2.sin_addr));
                printf("Client 2 port: %d\n", ntohs(client_addr2.sin_port));
            }
        }

        if (client1_set && client2_set)
        {
            // sent to client 1
            sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&client_addr, client_addr_len);
            // sent to client 2
            sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&client_addr2, client_addr_len2);
        }
    }

    close(sockfd);

    return 0;
}
