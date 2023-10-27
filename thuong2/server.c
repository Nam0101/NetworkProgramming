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
#define ACCOUNT_FILE "account.txt"

typedef struct Account
{
    char username[BUFF_SIZE];
    char password[BUFF_SIZE];
    int status;
} Account;

int check_account(char *username, char *password, Account *account_list, int num_accounts)
{
    for (int i = 0; i < num_accounts; i++)
    {
        if (strcmp(username, account_list[i].username) == 0)
        {
            if (strcmp(password, account_list[i].password) == 0)
            {
                if (account_list[i].status == 0)
                {
                    return 0; // OK
                }
                else
                {
                    return 1; // account blocked
                }
            }
            else
            {
                return -1; // wrong password
            }
        }
    }
    return -2; // account not found
}

void block_account(char *username, Account *account_list, int num_accounts)
{
    for (int i = 0; i < num_accounts; i++)
    {
        if (strcmp(username, account_list[i].username) == 0)
        {
            account_list[i].status = 1;
            break;
        }
    }
}

void activate_account(char *username, Account *account_list, int num_accounts)
{
    for (int i = 0; i < num_accounts; i++)
    {
        if (strcmp(username, account_list[i].username) == 0)
        {
            account_list[i].status = 0;
            break;
        }
    }
}

void change_password(char *username, char *new_password, Account *account_list, int num_accounts)
{
    for (int i = 0; i < num_accounts; i++)
    {
        if (strcmp(username, account_list[i].username) == 0)
        {
            strcpy(account_list[i].password, new_password);
            break;
        }
    }
}

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
    struct sockaddr_in server_addr, client_addr;

    // Step 1: Construct a TCP socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
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

    // Step 3: Listen request from client
    if (listen(server_sock, 10) == -1)
    {
        perror("\nError: ");
        exit(0);
    }

    // Load account data from file
    FILE *fp = fopen(ACCOUNT_FILE, "r");
    if (fp == NULL)
    {
        perror("\nError: ");
        exit(0);
    }
    int num_accounts = 0;
    Account account_list[BUFF_SIZE];
    while (fgets(buff, BUFF_SIZE, fp) != NULL)
    {
        sscanf(buff, "%s %s %d", account_list[num_accounts].username, account_list[num_accounts].password, &account_list[num_accounts].status);
        num_accounts++;
    }
    fclose(fp);

    // Step 4: Communicate with client
    while (1)
    {
        socklen_t sin_size = sizeof(struct sockaddr_in);
        if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &sin_size)) == -1)
        {
            perror("\nError: ");
            continue;
        }

        // Receive username and password from client
        bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
        if (bytes_received < 0)
        {
            perror("\nError: ");
            close(client_sock);
            continue;
        }
        buff[bytes_received] = '\0';
        char username[BUFF_SIZE], password[BUFF_SIZE];
        sscanf(buff, "%s %s", username, password);

        // Check account
        int check_result = check_account(username, password, account_list, num_accounts);
        if (check_result == 0)
        {
            // Account OK
            printf("login success\n");
            bytes_sent = send(client_sock, "OK", strlen("OK"), 0);
            if (bytes_sent < 0)
            {
                perror("\nError: ");
                close(client_sock);
                continue;
            }

            // Receive command from client
            while (1)
            {
                bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
                if (bytes_received < 0)
                {
                    perror("\nError: ");
                    close(client_sock);
                    break;
                }
                buff[bytes_received] = '\0';

                if (strcmp(buff, "bye") == 0)
                {
                    // Sign out
                    break;
                }
                else
                {
                    // Change password
                    if (strlen(buff) == 0)
                    {
                        bytes_sent = send(client_sock, "Insert password", strlen("Insert password"), 0);
                        if (bytes_sent < 0)
                        {
                            perror("\nError: ");
                            close(client_sock);
                            break;
                        }
                    }
                    else
                    {
                        int is_valid = 1;
                        for (int i = 0; i < strlen(buff); i++)
                        {
                            if (!isalnum(buff[i]))
                            {
                                is_valid = 0;
                                break;
                            }
                        }
                        if (!is_valid)
                        {
                            bytes_sent = send(client_sock, "Invalid password", strlen("Invalid password"), 0);
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                                close(client_sock);
                                break;
                            }
                        }
                        else
                        {
                            // Change password
                            char new_password[BUFF_SIZE];
                            strcpy(new_password, buff);
                            change_password(username, new_password, account_list, num_accounts);

                            // Send new password to client
                            unsigned char hash[SHA256_DIGEST_LENGTH];
                            SHA256((unsigned char *)new_password, strlen(new_password), hash);
                            char *hash_str = (char *)malloc(SHA256_DIGEST_LENGTH * 2 + 1);
                            for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
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
                            bytes_sent = send(client_sock, result, strlen(result), 0);
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                                close(client_sock);
                                break;
                            }
                            strcpy(result, digit_str);
                            bytes_sent = send(client_sock, result, strlen(result), 0);
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                                close(client_sock);
                                break;
                            }
                        }
                    }
                }
            }
        }
        else if (check_result == 1)
        {
            // Account blocked
            printf("login failed\n");
            bytes_sent = send(client_sock, "Account not ready", strlen("Account not ready"), 0);
            if (bytes_sent < 0)
            {
                perror("\nError: ");
                close(client_sock);
                continue;
            }
        }
        else
        {
            // Account not found or wrong password
            printf("login failed\n");
            bytes_sent = send(client_sock, "Not OK", strlen("Not OK"), 0);
            if (bytes_sent < 0)
            {
                perror("\nError: ");
                close(client_sock);
                continue;
            }

            // Block account if wrong password 3 times
            int wrong_password_count = 1;
            while (1)
            {
                bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
                if (bytes_received < 0)
                {
                    perror("\nError: ");
                    close(client_sock);
                    break;
                }
                buff[bytes_received] = '\0';

                if (strcmp(buff, "bye") == 0)
                {
                    // Sign out
                    break;
                }
                else
                {
                    if (strcmp(buff, password) == 0)
                    {
                        // Password OK
                        bytes_sent = send(client_sock, "OK", strlen("OK"), 0);
                        if (bytes_sent < 0)
                        {
                            perror("\nError: ");
                            close(client_sock);
                            break;
                        }
                        break;
                    }
                    else
                    {
                        // Wrong password
                        wrong_password_count++;
                        if (wrong_password_count == 3)
                        {
                            block_account(username, account_list, num_accounts);
                            bytes_sent = send(client_sock, "Account is blocked", strlen("Account blocked"), 0);
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                                close(client_sock);
                                break;
                            }
                            break;
                        }
                        else
                        {
                            bytes_sent = send(client_sock, "Not OK", strlen("Not OK"), 0);
                            if (bytes_sent < 0)
                            {
                                perror("\nError: ");
                                close(client_sock);
                                break;
                            }
                        }
                    }
                }
            }
        }

        close(client_sock);
    }

    close(server_sock);
    return 0;
}
