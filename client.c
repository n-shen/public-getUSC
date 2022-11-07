#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFSIZE 51
#define PORT_NUM_TCP_SERVERM 25448
#define IP_SERVERM "127.0.0.1"

/* get client port number */
int getMyPortNum(int sd, struct sockaddr_in my_address, socklen_t my_address_len)
{

    if (getsockname(sd, (struct sockaddr *)&my_address, &my_address_len) == -1)
    {
        perror("[Error] getsocket name");
        exit(-1);
    }

    return ntohs(my_address.sin_port);
}

int initClient(int *sd)
{
    struct sockaddr_in my_address;

    /* create a client socket */
    *sd = socket(AF_INET, SOCK_STREAM, 0);
    my_address.sin_family = AF_INET;
    my_address.sin_addr.s_addr = INADDR_ANY;
    my_address.sin_port = 0; // assign client port dynamically

    /* client: bind socket and ip */
    if (bind(*sd, (struct sockaddr *)&my_address, sizeof(my_address)) < 0)
    {
        perror("Client: binding error");
        exit(-1);
    }

    /* on screen */
    printf("The client is up and running.\n");

    /* return client port number to main */
    return getMyPortNum(*sd, my_address, sizeof(my_address));
}

void connServerM(int *sd)
{
    struct sockaddr_in serverM_address;

    /* create a ServerM socket */
    serverM_address.sin_family = AF_INET;
    serverM_address.sin_addr.s_addr = inet_addr(IP_SERVERM);
    serverM_address.sin_port = htons(PORT_NUM_TCP_SERVERM);

    /* connect to serverM */
    if (connect(*sd, (struct sockaddr *)&serverM_address, sizeof(struct sockaddr_in)) < 0)
    {
        close(*sd);
        perror("[Error] Client to ServerM - connecting stream socket");
        exit(-1);
    }
}

void sendUserAuth(int *sd, int type)
{
    int rc = 0;
    int sizeOfUserAuth;
    int converted_sizeOfUserAuth;
    char userAuth[BUFFSIZE];

    /* Ask client for Auth input */
    (type) ? (printf("Please enter the password: ")) : (printf("Please enter the username: "));
    fflush(stdout);
    fgets(userAuth, sizeof(userAuth), stdin);
    userAuth[strcspn(userAuth, "\n")] = 0;

    /* Send the size of input(string) to the server */
    sizeOfUserAuth = strlen(userAuth);
    converted_sizeOfUserAuth = ntohs(strlen(userAuth));
    if (write(*sd, &converted_sizeOfUserAuth, sizeof(converted_sizeOfUserAuth)) < 0)
        perror("Size of content send failed");

    /* Send message(string) to the server */
    if (write(*sd, userAuth, sizeOfUserAuth) < 0)
        perror("Content send failed");
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
    int sd, my_port_num;
    struct sockaddr_in my_address;

    my_port_num = initClient(&sd);
    printf("Client port: %d.\n", my_port_num);
    connServerM(&sd);
    commuServerM(&sd);

    return 0;
}
