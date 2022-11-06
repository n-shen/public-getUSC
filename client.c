#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFSIZE 100
#define PORT_NUM_TCP_SERVERM 25448

void initClient(int *sd)
{
    int portNumber = 8889; // DYNAMIC
    char serverIP[29] = "127.0.0.1";
    struct sockaddr_in server_address;

    /* create a socket */
    *sd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT_NUM_TCP_SERVERM);
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

void sendUserAuth(int *sd, int type)
{
    int rc = 0;
    int sizeOfUserAuth;
    int converted_sizeOfUserAuth;
    char userAuth[BUFFSIZE];

    /* ask user for message input */
    (type) ? (printf("Please enter the password: ")) : (printf("Please enter the username: "));
    fflush(stdout);
    fgets(userAuth, sizeof(userAuth), stdin);
    userAuth[strcspn(userAuth, "\n")] = 0;

    /* send size of input(string) to the server */
    sizeOfUserAuth = strlen(userAuth);
    converted_sizeOfUserAuth = ntohs(strlen(userAuth));
    rc = write(*sd, &converted_sizeOfUserAuth, sizeof(converted_sizeOfUserAuth));
    if (rc < 0)
        perror("send failed - 1");

    /* send message(string) to the server */
    rc = write(*sd, userAuth, sizeOfUserAuth);
    if (rc < 0)
        perror("send failed - 2");
}

void commuServerM(int *sd)
{

    /* CP_SESSION: send message to server repeatly */
CP_SESSION:
    sendUserAuth(sd, 0); // 0: username;
    sendUserAuth(sd, 1); // 1: password;

    goto CP_SESSION;
}

int main(int argc, char *argv[])
{
    int sd;

    initClient(&sd);
    commuServerM(&sd);

    return 0;
}
