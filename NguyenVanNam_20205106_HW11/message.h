#ifndef MESSAGE_H
#define MESSAGE_H
typedef enum
{
    LOGIN,
    LOGOUT,
    CHAT,
    UPLOADFILE,
    RESPONSE
} MessageType;
typedef struct
{
    char username[30];
    char password[30];
} LoginMessage;
typedef struct
{
    char username[30];
} LogoutMessage;
typedef struct
{
    char username[30];
    char message[256];
} ChatMessage;
typedef struct
{
    char username[30];
    char filename[30];
    int size;
} UploadFileMessage;
typedef struct
{
    int status;
    char message[256];
} ResponseMessage;
typedef struct
{
    MessageType type;
    union
    {
        LoginMessage login;
        LogoutMessage logout;
        ChatMessage chat;
        UploadFileMessage uploadfile;
        ResponseMessage response;
    } data;
} Message;
#endif