#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <pthread.h>

#define MAX_BUFFER_SIZE 2048

int sockfd;
struct sockaddr_in server_addr;

void *receive_message(void *arg)
{
    char buffer[MAX_BUFFER_SIZE];
    socklen_t server_addr_len = sizeof(server_addr);
    while (1)
    {
        int bytes_received = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &server_addr_len);
        if (bytes_received < 0)
        {
            perror("Error: ");
            close(sockfd);
            exit(1);
        }
        buffer[bytes_received] = '\0';
        printf("Reply from server: %s\n", buffer);
    }
}

void *send_message(void *arg)
{
    char message[MAX_BUFFER_SIZE];
    while (1)
    {
        printf("Enter a message to send to the server:\n");
        fgets(message, sizeof(message), stdin);
        if (message[0] == '@' || message[0] == '#'){
            close(sockfd);
            exit(1);
        }
        sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    }
}

int main(int argc, char *argv[])
{
    int PORT;
    char *SERVER_IP;
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

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    char *message = "Hello from client\n";
    sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));

    pthread_t send_thread;
    pthread_t receive_thread;

    pthread_create(&send_thread, NULL, send_message, NULL);
    pthread_create(&receive_thread, NULL, receive_message, NULL);

    pthread_join(send_thread, NULL);
    pthread_join(receive_thread, NULL);

    close(sockfd);

    return 0;
}
