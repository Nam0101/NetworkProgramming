#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/md5.h>
#define BACKLOG 1 /* Number of allowed connections */
#define BUFF_SIZE 8192
#define STRING_IN "S"
#define FILE_IN "F"
#define ERROR "Error Invalid String"
#define FILENAME "receive.jpg"
#define END1 "END1#"

// receive message from client
// input: client socket, message, message length
void receiveMessage(int client_sock, char *buff, int msg_len)
{
    int bytes_received = recv(client_sock, buff, msg_len, 0);
    if (bytes_received <= 0)
    {
        perror("\nError: ");
        return;
    }
    buff[bytes_received] = '\0';
}
// send message to client
// input: client socket, message, message length
void sendMessage(int client_sock, char *buff, int msg_len)
{
    int bytes_sent = send(client_sock, buff, msg_len, 0);
    if (bytes_sent <= 0)
    {
        perror("\nError: ");

        return;
    }
}
// hashing string to md5
// input: string
// output: md5 string
char *md5Hashing(char *str)
{
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5_CTX md5;
    MD5_Init(&md5);
    MD5_Update(&md5, str, strlen(str));
    MD5_Final(digest, &md5);
    char *md5Hash = (char *)malloc(33);
    for (int i = 0; i < 16; i++)
        sprintf(&md5Hash[i * 2], "%02x", (unsigned int)digest[i]);
    return md5Hash;
}
// get digit in md5 string
// input: md5 string
// output: digit string
char *digitInMD5(char *str)
{
    char *digit = (char *)malloc(BUFF_SIZE);
    int j = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] >= '0' && str[i] <= '9')
        {
            digit[j] = str[i];
            j++;
        }
    }
    digit[j] = '\0';
    return digit;
}

// check if character is letter
// input: character
// output: 1 if character is letter, 0 if not
int isLetter(char c)
{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}
// get letter in md5 string
// input: md5 string
// output: letter string
char *letterInMD5(char *str)
{
    char *letter = (char *)malloc(BUFF_SIZE);
    int j = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        if (isLetter(str[i]))
        {
            letter[j] = str[i];
            j++;
        }
    }
    letter[j] = '\0';
    return letter;
}

// check if character is valid
// input: character
// output: 1 if character is valid, 0 if not
int isValidChar(char c)
{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == ' ');
}
// check if string is valid
// input: string
// output: 1 if string is valid, 0 if not
int isValidString(char *str)
{
    int i = 0;
    while (i < strlen(str))
    {
        if (isValidChar(str[i]))
        {
            i++;
        }
        else
        {
            return 0;
        }
    }
    return 1;
}
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        return 0;
    }
    int PORT = atoi(argv[1]);
    int listen_sock, conn_sock; /* file descriptors */
    char recv_data[BUFF_SIZE];
    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */

    // Step 1: Construct a TCP socket to listen connection request
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    { /* calls socket() */
        perror("\nError: ");
        return 0;
    }
    socklen_t sin_size;
    // Step 2: Bind address to socket
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);              /* Remember htons() from "Conversions" section? =) */
    server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */
    if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
    { /* calls bind() */
        perror("\nError: ");
        return 0;
    }

    // Step 3: Listen request from client
    if (listen(listen_sock, BACKLOG) == -1)
    { /* calls listen() */
        perror("\nError: ");
        return 0;
    }

    // Step 4: Communicate with client
    while (1)
    {
        // accept request
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) < 0)
        {
            perror("\nError: ");
            printf("Can't accept connection\n");
            return 0;
        }

        printf("You got a connection from %s\n", inet_ntoa(client.sin_addr)); /* prints client's IP */

        // start conversation
        printf("Listening...\n");
        while (1)
        {
            // receives message from client
            receiveMessage(conn_sock, recv_data, BUFF_SIZE);
            if (strcmp(recv_data, "S") == 0)
            {
                receiveMessage(conn_sock, recv_data, BUFF_SIZE);
                printf("Receive string: %s\n", recv_data);
                // check if recv_data is contain a number or letter, if not, send error message
                if (strcmp(recv_data, END1) == 0)
                {
                    continue;
                }
                if (!isValidString(recv_data)) // in case recv_data is not valid, send error message
                {
                    sendMessage(conn_sock, ERROR, strlen(ERROR));
                }
                else
                {
                    // hashing recv_data to md5 and send to client
                    char *md5Hash = md5Hashing(recv_data);
                    char *digit = digitInMD5(md5Hash);
                    char *letter = letterInMD5(md5Hash);
                    char *result = (char *)malloc(BUFF_SIZE);
                    strcpy(result, digit);
                    strcat(result, " ");
                    strcat(result, letter);
                    sendMessage(conn_sock, result, strlen(result));
                }
            }
            else
            {
                printf("Receive file\n");
                // default file name is receive.jpg and locate in the same folder with server.c
                FILE *f = fopen(FILENAME, "wb");
                if (f == NULL)
                {
                    printf("Error: Can't open file\n");
                    return 0;
                }

                char *file_data = (char *)malloc(BUFF_SIZE);
                size_t bytesReceived;
                // receive file from client and write to file
                while (1)
                {
                    bytesReceived = recv(conn_sock, file_data, BUFF_SIZE, 0);
                    fwrite(file_data, 1, bytesReceived, f);
                    // File is devide to parts, each part is BUFF_SIZE bytes
                    // last part is less than BUFF_SIZE bytes
                    if (bytesReceived < BUFF_SIZE)
                    {
                        // file is received
                        break;
                    }
                }
                fclose(f);
            }
            memset(recv_data, 0, BUFF_SIZE);
        }
        // close connection
        close(conn_sock);
        close(listen_sock);
        return 0;
    }
}