#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFSIZE 51
#define PORT_NUM_SERVERM_TCP 25448
#define PORT_NUM_SERVERM_UDP 24448
#define PORT_NUM_SERVERC_UDP 21448
#define IP_SERVERM "127.0.0.1"
#define IP_SERVERC "127.0.0.1"

void commuClient(int *sd);

void initServerMUDP(int *sd_udp)
{
    struct sockaddr_in serverM_UDP_address;
    /* Create serverM socket. */
    *sd_udp = socket(AF_INET, SOCK_DGRAM, 0);

    serverM_UDP_address.sin_family = AF_INET;
    serverM_UDP_address.sin_port = htons(PORT_NUM_SERVERM_UDP);
    serverM_UDP_address.sin_addr.s_addr = INADDR_ANY;

    /* Bind ServerC socket and IP. */
    // check return code from bind
    if (bind(*sd_udp, (struct sockaddr *)&serverM_UDP_address, sizeof(serverM_UDP_address)) < 0)
    {
        perror("serverM Warning: UDP bind error");
        exit(-1);
    }
}

void initServerMTCP(int *sd)
{
    struct sockaddr_in serverM_address;

    /* Create serverM socket. */
    *sd = socket(AF_INET, SOCK_STREAM, 0);

    serverM_address.sin_family = AF_INET;
    serverM_address.sin_port = htons(PORT_NUM_SERVERM_TCP);
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

    /* read size */
    if (read(*connected_sd, &sizeOfUserAuth, sizeof(int)) <= 0)
    {
        printf("\n/*-------- Clinet disconneted! Waiting new clients... -----------*/\n");
        commuClient(sd);
    }

    /* read userAuth */

    if (read(*connected_sd, &buffer, ntohs(sizeOfUserAuth)) < 0)
    {
        printf("\n/*-------- Clinet disconneted! Waiting new clients... -----------*/\n");
        commuClient(sd);
    }

    /* Server - return result */
    strncpy(userAuth, buffer, BUFFSIZE);
}

void sendUserAuthFeedback(int *connected_sd)
{
    int authFeedbackType;

    /* Send the size of input(string) to the server */
    authFeedbackType = ntohs(101);
    if (write(*connected_sd, &authFeedbackType, sizeof(int)) < 0)
        perror("User Auth Feedback sent failed");
}

/* encrypt auth */
void encryptAuth(char *userAuth)
{
    char tmp[BUFFSIZE];
    memset(tmp, 0, BUFFSIZE);
    strncpy(tmp, userAuth, BUFFSIZE);

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

    strncpy(userAuth, tmp, BUFFSIZE);
}

void sendUserVerReq(char userName[BUFFSIZE], char userPsw[BUFFSIZE])
{
    printf("sending to ServerC.\n");
    // int rc = sendto(*sd_udp, &userName, strlen(userName), 0, (struct sockaddr *)serverM_UDP_address, sizeof(*serverM_UDP_address));
}
void verifyAuth(int *connected_sd, char *userName, char *userPsw)
{
    /* encrypt auth */
    encryptAuth(userName);
    encryptAuth(userPsw);
    printf("Encrypted: %s, %s.\n", userName, userPsw);
    // userName[strcspn(userName, "\n")] = 0;
    // int rc = sendto(sd_ServerC, &userName, strlen(userName), 0, (struct sockaddr *)&address_ServerC, sizeof(address_ServerC));
    /* send feedback */
    sendUserAuthFeedback(connected_sd);
}

void authProcess(int *sd, int *connected_sd)
{
    char userName[BUFFSIZE], userPsw[BUFFSIZE];

    recvUserAuth(sd, connected_sd, userName);
    recvUserAuth(sd, connected_sd, userPsw);
    printf("Received Auth: [%s,%s]\n", userName, userPsw);
    printf("The main server received the authentication for %s using TCP over port %d.\n", userName, PORT_NUM_SERVERM_TCP);

    verifyAuth(connected_sd, userName, userPsw);
}

void connectServerC(int *sd, struct sockaddr_in *server_address)
{
    /* connect to server */
    *sd = socket(AF_INET, SOCK_DGRAM, 0);

    server_address->sin_family = AF_INET;
    server_address->sin_port = htons(PORT_NUM_SERVERC_UDP);
    server_address->sin_addr.s_addr = INADDR_ANY;
}

void commuClient(int *sd)
{
    int connected_sd, connected_client_port;
    struct sockaddr_in client_address;
    socklen_t client_address_len;

    /* LOOP1 - listen to incoming client, limit: one student */
    listen(*sd, 1);
    connected_sd = accept(*sd, (struct sockaddr *)&client_address, &client_address_len);

    int sd_ServerC;
    struct sockaddr_in address_ServerC;
    connectServerC(&sd_ServerC, &address_ServerC);

/* LOOP2 - receive message from connected clients */
CP_SESSION:
    authProcess(sd, &connected_sd);

    goto CP_SESSION;
}

int main(int argc, char *argv[])
{
    int sd_tcp, sd_udp; /* socket descriptor */

    initServerMTCP(&sd_tcp);
    initServerMUDP(&sd_udp);
    commuClient(&sd_tcp);

    return 0;
}
