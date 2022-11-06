#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFSIZE 100

void commuClient(int *sd);

void initServerM(int *sd)
{
    int portNumber = 25448;
    struct sockaddr_in server_address;
    int rc; /* return code from recvfrom */

    /* create a socket */
    *sd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portNumber);
    server_address.sin_addr.s_addr = INADDR_ANY;

    /* bind socket and ip */
    rc = bind(*sd, (struct sockaddr *)&server_address, sizeof(server_address));
    if (rc < 0)
    {
        perror("serverM Warning: bind error");
        exit(-1);
    }

    printf("The main server is up and running.\n");
}

void recvUserName(int *sd, int *connected_sd)
{

    /* Server - Read file name and filename size*/
    int rc;
    int sizeOfUserName;
    char buffer[BUFFSIZE];
    memset(buffer, 0, BUFFSIZE);
    char *ptr = buffer;

    /* read size */
    rc = read(*connected_sd, &sizeOfUserName, sizeof(int));
    if (rc <= 0)
    {
        printf("\n\n/*-------- Client Status: Clinet disconneted! Waiting new clients... -----------*/\n");
        commuClient(sd);
    }

    sizeOfUserName = ntohs(sizeOfUserName);
    /* read userName */
    rc = read(*connected_sd, ptr, sizeOfUserName);
    if (rc <= 0)
    {
        perror("reveive error");
        exit(-1);
    }

    /* Server - Print result */
    char userName[BUFFSIZE];
    strncpy(userName, buffer, 100);
    printf("[SERVER Notice] Received username: \"%s\"\n", userName);
}

/* ServerM: read userPsw from client */
void recvUserPsw(int *sd, int *connected_sd)
{
    int rc;
    int sizeOfUserPsw;
    char buffer[BUFFSIZE];
    memset(buffer, 0, BUFFSIZE);
    char *ptr = buffer;

    /* read size */
    rc = read(*connected_sd, &sizeOfUserPsw, sizeof(int));
    if (rc <= 0)
    {
        printf("\n\n/*-------- Client Status: Clinet disconneted! Waiting new clients... -----------*/\n");
        commuClient(sd);
    }

    sizeOfUserPsw = ntohs(sizeOfUserPsw);
    /* read userName */
    rc = read(*connected_sd, ptr, sizeOfUserPsw);
    if (rc <= 0)
    {
        perror("reveive error");
        exit(-1);
    }

    /* Server - Print result */
    char userPsw[BUFFSIZE];
    strncpy(userPsw, buffer, BUFFSIZE);
    printf("[SERVER Notice] Received password: \"%s\"\n", userPsw);
}

void commuClient(int *sd)
{
    int connected_sd; /* socket descriptor */
    int rc;           /* return code from recvfrom */
    struct sockaddr_in from_address;
    socklen_t fromLength;

    /* LOOP1 - listen to incoming client, limit: one student */
    listen(*sd, 1);
    connected_sd = accept(*sd, (struct sockaddr *)&from_address, &fromLength);
    printf("[SERVER Notice] Clinet conneted! Listening...\n");
/* LOOP2 - receive message from connected clients */
CP_SESSION:
    recvUserName(sd, &connected_sd);
    recvUserPsw(sd, &connected_sd);

    goto CP_SESSION;
}

int main(int argc, char *argv[])
{
    int sd; /* socket descriptor */

    initServerM(&sd);
    commuClient(&sd);

    return 0;
}