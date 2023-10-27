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
    int bytes_sent, bytes_received;
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
    socklen_t server_addr_len = sizeof(server_addr);
    sendto(client_sock, "C1", strlen("C1"), 0, (const struct sockaddr *)&server_addr, server_addr_len);

    // Step 3: Communicate with server
    while (1)
    {
        char username[BUFF_SIZE];
        char password[BUFF_SIZE];

        // Get username
        printf("Enter username: ");
        fgets(username, BUFF_SIZE, stdin);

        // Get password
        printf("Enter password: ");
        fgets(password, BUFF_SIZE, stdin);

        // Remove newline characters from username and password
        username[strcspn(username, "\n")] = 0;
        password[strcspn(password, "\n")] = 0;

        // Send username and password to server
        char message[BUFF_SIZE];
        sprintf(message, "%s:%s", username, password);
        bytes_sent = sendto(client_sock, message, strlen(message), 0, (struct sockaddr *)&server_addr, server_addr_len);
        if (bytes_sent < 0)
        {
            perror("Error: ");
            close(client_sock);
            return 0;
        }

        // Receive response from server
        bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&server_addr, &server_addr_len);
        if (bytes_received < 0)
        {
            perror("Error: ");
            close(client_sock);
            return 0;
        }
        buff[bytes_received] = '\0';

        // Check if the user wants to exit
        if (username[0] == '@' || username[0] == '#')
        {
            break;
        }

        // Check server response
        if (strcmp(buff, "OK") == 0)
        {
            printf("Login successful!\n");

            // Ask user for new password
            printf("Enter new password: ");
            fgets(password, BUFF_SIZE, stdin);
            password[strcspn(password, "\n")] = 0;

            // If new password is empty, exit
            if (strlen(password) == 0)
            {
                break;
            }

            // If new password is "bye", exit
            if (strcmp(password, "bye") == 0)
            {
                break;
            }

            // Send new password to server
            sprintf(message, "%s:%s", username, password);
            bytes_sent = sendto(client_sock, message, strlen(message), 0, (struct sockaddr *)&server_addr, server_addr_len);
            if (bytes_sent < 0)
            {
                perror("Error: ");
                close(client_sock);
                return 0;
            }
        }
        else
        {
            printf("Login failed. Please try again.\n");
        }
    }

    close(client_sock);
    return 0;
}
