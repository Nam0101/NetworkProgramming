#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <openssl/sha.h>
#include <stdlib.h>
#define BUFF_SIZE 1024

int main(int argc, char *argv[])
{
    int client_sock;
    char buff[BUFF_SIZE];
    struct sockaddr_in server_addr;
    int bytes_sent;
    socklen_t sin_size;
    char *ip_address;
    int port_number;

    if (argc != 3)
    {
        printf("Usage: %s IPAddress PortNumber\n", argv[0]);
        return 0;
    }

    ip_address = argv[1];
    port_number = atoi(argv[2]);

    // Step 1: Construct a UDP socket
    if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("\nError: ");
        exit(0);
    }

    // Step 2: Define the address of the server
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_number);
    server_addr.sin_addr.s_addr = inet_addr(ip_address);

    // Step 3: Communicate with server
    socklen_t server_addr_len = sizeof(server_addr);
    sendto(client_sock, "Client2", strlen("Client2"), 0, (const struct sockaddr *)&server_addr, server_addr_len);
    while(1){
        bytes_sent = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&server_addr, &sin_size);
        if (bytes_sent < 0)
        {
            perror("Error: ");
            close(client_sock);
            return 0;
        }
        buff[bytes_sent] = '\0';
        printf("Server message: %s\n", buff);
    }
    close(client_sock);
}