#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
Node is a struct that contains a username, a password, a status (0 for locked, 1 for unlocked), and a pointer to the next node.
*/
typedef struct node
{
    char *userName;
    char *password;
    short status;
    struct node *next;
} node_t;

/*
Struct list is a linked list of nodes, with a head and a tail.
*/
typedef struct
{
    node_t *head;
    node_t *tail;
} list_t;

/* This function frees the memory allocated for a node. */
void freeNode(node_t *node)
{
    free(node->userName);
    free(node->password);
    free(node);
}

/* This function frees the memory allocated for a linked list. */
void freeList(list_t *list)
{
    node_t *cur = list->head;
    while (cur != NULL)
    {
        node_t *temp = cur;
        cur = cur->next;
        freeNode(temp);
    }
    free(list);
}

/* This function prints the contents of a linked list. */
void printList(list_t *list)
{
    node_t *cur = list->head;
    while (cur != NULL)
    {
        printf("%s %s %hd\n", cur->userName, cur->password, cur->status);
        cur = cur->next;
    }
}

/* This function adds a new node to the tail of a linked list. */
void addTail(list_t *list, node_t *newNode)
{
    if (list->head == NULL)
    {
        list->head = newNode;
        list->tail = newNode;
    }
    else
    {
        list->tail->next = newNode;
        list->tail = newNode;
    }
}

/* This function searches for a node with a given username in a linked list. */

/* This function checks if an account with a given username is locked in a linked list. */
int isAccountLocked(list_t *list, char *userName)
{
    node_t *cur = list->head;
    while (cur != NULL)
    {
        if (strcmp(cur->userName, userName) == 0)
        {
            if (cur->status == 0)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
        cur = cur->next;
    }
    return 0;
}

list_t *createList()
{
    list_t *list = (list_t *)malloc(sizeof(list_t));
    list->head = NULL;
    list->tail = NULL;
    return list;
}

void readFile(list_t *list, const char *fileName)
{
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL)
    {
        perror("Error: ");
        close(fp);
        exit(1);
    }
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, fp)) != -1)
    {
        char *temp = strdup(line);
        char *userName = strtok(temp, " ");
        char *password = strtok(NULL, " ");
        char *status = strtok(NULL, " ");
        node_t *newNode = (node_t *)malloc(sizeof(node_t));
        newNode->userName = strdup(userName);
        newNode->password = strdup(password);
        newNode->status = atoi(status);
        newNode->next = NULL;
        if (list->head == NULL)
        {
            list->head = newNode;
            list->tail = newNode;
        }
        else
        {
            list->tail->next = newNode;
            list->tail = newNode;
        }
    }
    fclose(fp);
    if (line)
        free(line);
}


