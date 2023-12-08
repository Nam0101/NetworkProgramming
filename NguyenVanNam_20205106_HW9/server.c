#include <openssl/aes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
int PORT;
#define AES_KEY_SIZE 256
unsigned char *aes_key = (unsigned char *)"20205106";
pthread_mutex_t c_mutex = PTHREAD_MUTEX_INITIALIZER;
#define MAX_CLIENTS 100

int client_sockets[MAX_CLIENTS];
int num_clients = 0;

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
        perror("Memory allocation error in decrypt_message");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < blocks; i++)
    {
        AES_decrypt((const unsigned char *)(encrypted_input + i * AES_BLOCK_SIZE),
                    decrypted_buffer + i * AES_BLOCK_SIZE, &aes);
    }
    size_t unpadded_length = output_length - decrypted_buffer[output_length - 1];
    decrypted_output = malloc(unpadded_length + 1);
    if (!(*decrypted_output))
    {
        free(decrypted_buffer);
        perror("Memory allocation error in decrypt_message");
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
        perror("Memory allocation error in encrypt_message");
        exit(EXIT_FAILURE);
    }
    // Pad the input if necessary
    size_t padded_length = blocks * AES_BLOCK_SIZE;
    unsigned char *padded_input = (unsigned char *)malloc(padded_length);
    if (!padded_input)
    {
        free(encrypted_buffer);
        perror("Memory allocation error in encrypt_message");
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

void broadcast_message(int client_sock, char *message)
{
    for (int i = 0; i < num_clients; i++)
    {
        if (client_sockets[i] != client_sock)
        {
            send(client_sockets[i], message, strlen(message), 0);
        }
    }
}

void add_client(int client_sockfd)
{
    client_sockets[num_clients] = client_sockfd;
    num_clients++;
}

void remove_client(int client_sockfd)
{
    int i;
    for (i = 0; i < num_clients; i++)
    {
        if (client_sockets[i] == client_sockfd)
        {
            break;
        }
    }
    for (; i < num_clients - 1; i++)
    {
        client_sockets[i] = client_sockets[i + 1];
    }
    num_clients--;
}
// create a TCP socket
int create_socket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        exit(1);
    }
    return sockfd;
}

// create server address
struct sockaddr_in create_server_addr()
{
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    return server_addr;
}

// bind socket to server address and port
void bind_socket(int sockfd, struct sockaddr_in server_addr)
{
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error binding");
        exit(1);
    }
    if (listen(sockfd, 5) < 0)
    {
        perror("Error listening");
        exit(1);
    }
}
void recvice_message(int sockfd, char *message)
{
    ssize_t num_bytes = recv(sockfd, message, sizeof(message), 0);
    if (num_bytes == 0)
    {
        printf("Client closed connection\n");
        close(sockfd);
        pthread_mutex_lock(&c_mutex);
        remove_client(sockfd);
        pthread_mutex_unlock(&c_mutex);
        return;
    }
    else if (num_bytes == -1)
    {
        close(sockfd);
        pthread_mutex_lock(&c_mutex);
        remove_client(sockfd);
        pthread_mutex_unlock(&c_mutex);
        return;
    }
    // Null-terminate the message to safely use it as a string
}

void *handle_client(void *arg)
{
    int client_sockfd = *(int *)arg;
    char *message = (char *)malloc(sizeof(char) * 1024);
    while (1)
    {
        recvice_message(client_sockfd, message);
        broadcast_message(client_sockfd, message);
        memset(message, 0, sizeof(char) * 1024);
    }
    return NULL;
}
int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    int sockfd;
    if (argc < 2)
    {
        printf("Please enter port number\n");
        printf("Usage: ./server <port>\n");
        exit(1);
    }
    PORT = atoi(argv[1]);
    // Create a TCP socket
    sockfd = create_socket();

    // Create server address
    server_addr = create_server_addr();

    // Bind the socket to the specified port
    bind_socket(sockfd, server_addr);

    printf("TCP server is listening on port %d...\n", PORT);

    while (1)
    {
        socklen_t client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sockfd < 0)
        {
            perror("Error accepting connection");
            continue;
        }
        pthread_mutex_lock(&c_mutex);
        add_client(client_sockfd);
        printf("Number of clients: %d\n", num_clients);
        pthread_mutex_unlock(&c_mutex);
        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, &client_sockfd);
    }
}