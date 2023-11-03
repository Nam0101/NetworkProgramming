#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#define ERROR "Error Invalid String"
#define BUFF_SIZE 8192
#define END1 "END1#"
#define FILENAME "image.jpg"
void printMenu()
{
    printf("MENU\n");
    printf("--------------------------------------\n");
    printf("1. Gui xau bat ky\n");
    printf("2. Gui noi dung mot file\n");
}

void sendMessage(int client_sock, char *buff, int msg_len)
{
    int bytes_sent = send(client_sock, buff, msg_len, 0);
    if (bytes_sent <= 0)
    {
        printf("\nConnection closed!\n");
        return;
    }
    printf("Sent: %d\n", bytes_sent);
}
void receiveMessage(int client_sock, char *buff, int msg_len)
{
    int bytes_received = recv(client_sock, buff, msg_len, 0);
    if (bytes_received <= 0)
    {
        printf("\nConnection closed!\n");
        return;
    }
    buff[bytes_received] = '\0';
}

void sendFile(int client_sock, char *fileName)
{
    FILE *f = fopen(fileName, "rb");
    if (f == NULL)
    {
        printf("Error: File not found\n");
        return;
    }
    // đọc file
    char file_data[BUFF_SIZE];
    size_t bytesRead;
    while ((bytesRead = fread(file_data, sizeof(char), BUFF_SIZE, f)) > 0)
    {
        sendMessage(client_sock, file_data, bytesRead);
        memset(file_data, 0, BUFF_SIZE);
    }
    fclose(f);
}




int main(int argc, char *argv[])
{
    int SERVER_PORT;
    char *SERVER_ADDR;
    if (argc != 3)
    {
        printf("Usage: %s <server address> <port>\n", argv[0]);
        return 0;
    }
    SERVER_ADDR = argv[1];
    SERVER_PORT = atoi(argv[2]);
    int client_sock;
    char buff[BUFF_SIZE];
    struct sockaddr_in server_addr; /* server's address information */
    int msg_len;

    // Step 1: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    // Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    // Step 3: Request to connect server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("\nError!Can not connect to sever! Client exit imediately! ");
        return 0;
    }
    int choise, c, check = 1;
    // Step 4: Communicate with server
    while (1)
    {
        printMenu();
        choise = 0;
        scanf("%d", &choise);
        if (check) // check for clear buffer
            getchar();

        switch (choise)
        {
        case 1:
            while (1)
            {
                printf("Nhap xau ky tu (Enter de ket thuc): ");
                memset(buff, 0, BUFF_SIZE);
                sendMessage(client_sock, "S", strlen("S"));
                fgets(buff, BUFF_SIZE, stdin);
                msg_len = strlen(buff);
                if (buff[msg_len - 1] == '\n')
                {
                    buff[msg_len - 1] = '\0';
                }

                if (msg_len == 1)
                {
                    check = 0;
                    sendMessage(client_sock, END1, strlen(END1));
                    printf("Goodbye\n");
                    break; // Thoát vòng lặp khi chuỗi rỗng được nhập
                }

                sendMessage(client_sock, buff, msg_len);
                receiveMessage(client_sock, buff, BUFF_SIZE);
                if (strcmp(buff, ERROR) == 0)
                {
                    printf("Error: Invalid String\n");
                }
                else
                {
                    printf("Reply from server: %s\n", buff);
                }
            }
            break;

        case 2:
            check = 0;
            while ((c = getchar()) != '\n' && c != EOF)
            {
            }
            memset(buff, 0, BUFF_SIZE);
            sendMessage(client_sock, "F", strlen("F"));
            sendFile(client_sock, FILENAME);
            printf("Send file success\n");
            break;

        default:
            printf("Invalid choice\n");
            break;
        }
    }
    // Step 4: Close socket
    close(client_sock);
    return 0;
}
