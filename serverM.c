#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFSIZE 51
#define PORT_NUM_TCP 25448
#define IP_SERVERM "127.0.0.1"

void commuClient(int *sd);

void initServerM(int *sd)
{
    struct sockaddr_in serverM_address;

    /* Create serverM socket. */
    *sd = socket(AF_INET, SOCK_STREAM, 0);

    serverM_address.sin_family = AF_INET;
    serverM_address.sin_port = htons(PORT_NUM_TCP);
    serverM_address.sin_addr.s_addr = INADDR_ANY;

    /* Bind ServerM socket and IP. */
    // check return code from bind
    if (bind(*sd, (struct sockaddr *)&serverM_address, sizeof(serverM_address)) < 0)
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
        printf("\n/*-------- Clinet disconneted! Waiting new clients... -----------*/\n");
        commuClient(sd);
    }

    /* Server - return result */
    strncpy(userAuth, buffer, BUFFSIZE);
}

void sendUserAuthFeedback(int connected_sd)
{
    int authFeedbackType;

    /* Send the size of input(string) to the server */
    authFeedbackType = ntohs(101);
    if (write(connected_sd, &authFeedbackType, sizeof(int)) < 0)
        perror("User Auth Feedback sent failed");
}

/* encrypt auth */
void encryptAuth(char *userAuth[BUFFSIZE])
{
    char tmp[BUFFSIZE];
    strcpy(tmp, *userAuth);
    int i;
    for (i = 0; i < strlen(tmp); i++)
    {
        if (tmp[i] >= 'A' && tmp[i] <= 'Z')
            tmp[i] = 65 + (tmp[i] - 61) % 26;
        else if (tmp[i] >= 'a' && tmp[i] <= 'z')
            tmp[i] = 97 + (tmp[i] - 93) % 26;
        else if (tmp[i] >= '0' && tmp[i] <= '9')
            tmp[i] = 48 + (tmp[i] - 44) % 10;
    }
    strcpy(*userAuth, tmp);
}

void verifyAuth(int connected_sd, char userName[BUFFSIZE], char userPsw[BUFFSIZE])
{
    /* encrypt auth */
    encryptAuth(&userName);
    encryptAuth(&userPsw);
    printf("encrypted: %s, %s.\n", userName, userPsw);

    /* send feedback */
    sendUserAuthFeedback(connected_sd);
}

void commuClient(int *sd)
{
    int connected_sd, connected_client_port;
    struct sockaddr_in client_address;
    socklen_t client_address_len;

    char userName[BUFFSIZE], userPsw[BUFFSIZE];

    /* LOOP1 - listen to incoming client, limit: one student */
    listen(*sd, 1);
    connected_sd = accept(*sd, (struct sockaddr *)&client_address, &client_address_len);

/* LOOP2 - receive message from connected clients */
CP_SESSION:

    recvUserAuth(sd, &connected_sd, userName);
    recvUserAuth(sd, &connected_sd, userPsw);
    printf("Received Auth: [%s,%s]\n", userName, userPsw);
    printf("The main server received the authentication for %s using TCP over port %d.\n", userName, PORT_NUM_TCP);

    verifyAuth(connected_sd, userName, userPsw);

    goto CP_SESSION;
}

int main(int argc, char *argv[])
{
    int sd; /* socket descriptor */

    initServerM(&sd);
    commuClient(&sd);

    return 0;
}
