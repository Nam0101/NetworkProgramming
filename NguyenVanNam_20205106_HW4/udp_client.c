#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

int main(int argc, char *argv[])
{
    int sockfd;
    int PORT;
    char* SERVER_IP;
    if(argc <3){
        printf("Please enter port number and server IP\n");
        printf("Usage: ./client <server_ip> <port>\n");
        exit(1);
    }
    PORT = atoi(argv[2]);
    SERVER_IP = argv[1];
    struct sockaddr_in server_addr;
    char message[1024];

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

    while (1) {
        printf("Enter a message to send to the server: ");
        fgets(message, sizeof(message), stdin);
        if(message[0] == '@' || message[0] == '#')
            break;
        sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    }
    char buff[1024];
    int server_addr_len = sizeof(server_addr);
    int  bytes_received = recvfrom(sockfd, (char *)buff, 1024, 0, (struct sockaddr *)&server_addr, &server_addr_len);
    if (bytes_received < 0)
    {
        perror("Error: ");
        close(sockfd);
        return 0;
    }
    buff[bytes_received] = '\0';
    printf("Reply from server: %s", buff);

    close(sockfd);

    return 0;
}
