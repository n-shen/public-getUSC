#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

void initClient(int *sd)
{
    int portNumber = 25448;
    char serverIP[29] = "127.0.0.1";
    struct sockaddr_in server_address;

    /* create a socket */
    *sd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portNumber);
    server_address.sin_addr.s_addr = inet_addr(serverIP);

    /* connect to serverM */
    if (connect(*sd, (struct sockaddr *)&server_address, sizeof(struct sockaddr_in)) < 0)
    {
        close(*sd);
        perror("Client Warning: error connecting stream socket!");
        exit(-1);
    }
    printf("The client is up and running.\n");
}

void commuServerM(int *sd)
{
    int rc = 0;
    int sizeOfUserName;
    int converted_sizeOfUserName;
    char userName[100];

    /* Loop1: send message to server repeatly */
LOOP1:
    /* ask user for message input */
    printf("Please enter the username: ");
    fflush(stdout);
    fgets(userName, sizeof(userName), stdin);
    userName[strcspn(userName, "\n")] = 0;

    /* if input == 'STOP', close client */
    if (strcmp(userName, "STOP") == 0)
    {
        printf("Client Closed!\n");
        exit(2);
    }

    /* send size of input(string) to the server */
    sizeOfUserName = strlen(userName);
    converted_sizeOfUserName = ntohs(sizeOfUserName);
    rc = write(*sd, &converted_sizeOfUserName, sizeof(converted_sizeOfUserName));
    if (rc < 0)
        perror("write-1");

    /* send message(string) to the server */
    rc = write(*sd, userName, sizeOfUserName);
    printf("Client: String sent(\"%s\")!\n", userName);
    if (rc < 0)
        perror("write-2");

    goto LOOP1;
}

int main(int argc, char *argv[])
{
    int sd;

    initClient(&sd);
    commuServerM(&sd);

    return 0;
}