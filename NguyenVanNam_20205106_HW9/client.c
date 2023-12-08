#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/aes.h>
#include <pthread.h>
#define MAX_BUFFER_SIZE 4096
// socket file descriptor
#define AES_KEY_SIZE 256
unsigned char *aes_key = (unsigned char *)"20205106";
int sockfd;
struct sockaddr_in server_addr;
void *sendMessage(void *arg);
void *getMessage(void *arg);
char *name;
// AES
void decrypt_message(const char *encrypted_input, char **decrypted_output)
{
    AES_KEY aes;
    AES_set_decrypt_key(aes_key, AES_KEY_SIZE, &aes);
    size_t input_length = strlen(encrypted_input);
    int blocks = input_length / AES_BLOCK_SIZE;
    size_t output_length = blocks * AES_BLOCK_SIZE;

    unsigned char *decrypted_buffer = (unsigned char *)malloc(output_length);
    if (!decrypted_buffer)
    {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    // Giải mã từng khối dữ liệu
    for (int i = 0; i < blocks; i++)
    {
        AES_decrypt((const unsigned char *)(encrypted_input + i * AES_BLOCK_SIZE),
                    decrypted_buffer + i * AES_BLOCK_SIZE, &aes);
    }
    // Loại bỏ phần padding
    size_t unpadded_length = output_length - decrypted_buffer[output_length - 1];
    *decrypted_output = (char *)malloc(unpadded_length + 1);
    if (!(*decrypted_output))
    {
        free(decrypted_buffer);
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    memcpy(*decrypted_output, decrypted_buffer, unpadded_length);
    (*decrypted_output)[unpadded_length] = '\0';
    free(decrypted_buffer);
}
char *encrypt_message(const char *input)
{
    AES_KEY aes;
    AES_set_encrypt_key(aes_key, AES_KEY_SIZE, &aes);
    size_t input_length = strlen(input);
    int blocks = (input_length + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE;
    size_t output_length = blocks * AES_BLOCK_SIZE;
    unsigned char *encrypted_buffer = (unsigned char *)malloc(output_length);
    if (!encrypted_buffer)
    {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    // Pad the input if necessary
    size_t padded_length = blocks * AES_BLOCK_SIZE;
    unsigned char *padded_input = (unsigned char *)malloc(padded_length);
    if (!padded_input)
    {
        free(encrypted_buffer);
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    memcpy(padded_input, input, input_length);
    memset(padded_input + input_length, AES_BLOCK_SIZE - input_length % AES_BLOCK_SIZE, AES_BLOCK_SIZE - input_length % AES_BLOCK_SIZE);

    // Encrypt each block
    for (int i = 0; i < blocks; i++)
    {
        AES_encrypt(padded_input + i * AES_BLOCK_SIZE, encrypted_buffer + i * AES_BLOCK_SIZE, &aes);
    }

    free(padded_input);

    return (char *)encrypted_buffer;
}
void *sendMessage(void *arg)
{
    char *message = (char *)malloc(MAX_BUFFER_SIZE);
    while (1)
    {
        printf("Enter message:\n");
        fgets(message, MAX_BUFFER_SIZE, stdin);
        message[strlen(message) - 1] = '\0';
        // add name to message
        char *temp = (char *)malloc(MAX_BUFFER_SIZE);
        strcpy(temp, name);
        strcat(temp, ": ");
        strcat(temp, message);
        strcpy(message, temp);
        free(temp);
        char *encrypted_message = encrypt_message(message);
        send(sockfd, encrypted_message, strlen((char *)encrypted_message), 0);
        if (strcmp(message, "exit") == 0)
        {
            break;
        }
        memset(message, 0, strlen(message));
    }
    return NULL;
}

void *getMessage(void *arg)
{
    char *message = (char *)malloc(MAX_BUFFER_SIZE);
    if (!message)
    {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    while (1)
    {
        memset(message, 0, strlen(message));
        int bytes_received = recv(sockfd, message, MAX_BUFFER_SIZE, 0);
        if (bytes_received < 0)
        {
            perror("Error: ");
            break;
        }
        else if (bytes_received == 0)
        {
            printf("Connection closed\n");
            break;
        }
        else
        {
            char *decrypted_message = (char *)malloc(MAX_BUFFER_SIZE);
            decrypt_message(message, &decrypted_message);
            printf("%s\n", decrypted_message);
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int PORT;
    char *SERVER_IP;
    char *message = (char *)malloc(MAX_BUFFER_SIZE);
    if (argc < 3)
    {
        printf("Please enter port number and server IP\n");
        printf("Usage: ./client <server_ip> <port>\n");
        exit(1);
    }
    PORT = atoi(argv[2]);
    SERVER_IP = argv[1];

    // Create a UDP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        close(sockfd);
        exit(1);
    }
    // get connect to server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error connecting");
        close(sockfd);
        exit(1);
    }
    printf("Enter your name: ");
    fgets(message, MAX_BUFFER_SIZE, stdin);
    name = (char *)malloc(strlen(message));
    message[strlen(message) - 1] = '\0';
    strcpy(name, message);
    printf("Welcome %s\n", name);
    pthread_t send_thread, recv_thread;
    pthread_create(&send_thread, NULL, sendMessage, NULL);
    pthread_create(&recv_thread, NULL, getMessage, NULL);
    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);

    close(sockfd);
    free(message);

    return 0;
}
