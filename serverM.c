#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFSIZE 100
#define PORT_NUM_TCP 25448
#define IP_SERVERM "127.0.0.1"

void commuClient(int *sd);

void initServerM(int *sd)
{
    struct sockaddr_in server_address;

    /* Create serverM socket. */
    *sd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT_NUM_TCP);
    server_address.sin_addr.s_addr = INADDR_ANY;

    /* Bind ServerM socket and IP. */
    // check return code from recvfrom
    if (bind(*sd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("serverM Warning: bind error");
        exit(-1);
    }

    printf("The main server is up and running.\n");
}

/* Server - Receive client's Auth input */
void recvUserAuth(int *sd, int *connected_sd, char *userAuth)
{
    int sizeOfUserAuth;
    char buffer[BUFFSIZE];
    memset(buffer, 0, BUFFSIZE);
    char *ptr = buffer;

    /* read size */
    if (read(*connected_sd, &sizeOfUserAuth, sizeof(int)) <= 0)
    {
        printf("\n/*-------- Clinet disconneted! Waiting new clients... -----------*/\n");
        commuClient(sd);
    }

    /* read userAuth */
    sizeOfUserAuth = ntohs(sizeOfUserAuth);
    if (read(*connected_sd, ptr, sizeOfUserAuth) < 0)
    {
        perror("[ERROR] Reveiving");
        exit(-1);
    }

    /* Server - return result */
    strncpy(userAuth, buffer, BUFFSIZE);
}

void commuClient(int *sd)
{
    int connected_sd; /* socket descriptor */
    struct sockaddr_in from_address;
    socklen_t fromLength;

    char userName[BUFFSIZE];
    char userPsw[BUFFSIZE];

    /* LOOP1 - listen to incoming client, limit: one student */
    listen(*sd, 1);
    connected_sd = accept(*sd, (struct sockaddr *)&from_address, &fromLength);
    printf("[SERVER Notice] Clinet conneted! Listening...\n");

/* LOOP2 - receive message from connected clients */
CP_SESSION:

    recvUserAuth(sd, &connected_sd, userName);
    recvUserAuth(sd, &connected_sd, userPsw);
    printf("Received Auth: [%s,%s]\n", userName, userPsw);
    printf("The main server received the authentication for %s using TCP over port %d.\n", userName, PORT_NUM_TCP);

    goto CP_SESSION;
}

int main(int argc, char *argv[])
{
    int sd; /* socket descriptor */

    initServerM(&sd);
    commuClient(&sd);

    return 0;
}