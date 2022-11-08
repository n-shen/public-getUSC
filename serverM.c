#include "header.h"

void commuClient(int *sd_tcp, int *sd_udp);

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
void recvUserAuth(int *sd_tcp, int *sd_udp, int *connected_sd_tcp, char *userAuth)
{
    int sizeOfUserAuth;
    char buffer[BUFFSIZE];
    memset(buffer, 0, BUFFSIZE);

    /* read size and buffer */
    if ((read(*connected_sd_tcp, &sizeOfUserAuth, sizeof(int)) <= 0) || (read(*connected_sd_tcp, &buffer, ntohs(sizeOfUserAuth)) < 0))
    {
        printf("\n/*-------- Clinet disconneted! Waiting new clients... -----------*/\n");
        commuClient(sd_tcp, sd_udp);
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

void verifyAuth(struct User_auth *newUser, int *sd_udp, struct sockaddr_in *address_ServerC)
{
    /* encrypt auth */
    encryptAuth(newUser->userName);
    encryptAuth(newUser->userPsw);
    printf("Encrypted: %s, %s.\n", newUser->userName, newUser->userPsw);
    if (sendto(*sd_udp, (struct User_auth *)newUser, (1024 + sizeof(newUser)), 0, (struct sockaddr *)address_ServerC, sizeof(*address_ServerC)) <= 0)
        perror("UDP send user auth req failed");

    
}

void authProcess(int *sd_tcp, int *connected_sd_tcp, int *sd_udp, struct sockaddr_in *address_ServerC)
{
    struct User_auth newUser;

    recvUserAuth(sd_tcp, sd_udp, connected_sd_tcp, newUser.userName);
    recvUserAuth(sd_tcp, sd_udp, connected_sd_tcp, newUser.userPsw);
    printf("Received Auth: [%s,%s]\n", newUser.userName, newUser.userPsw);
    printf("The main server received the authentication for %s using TCP over port %d.\n", newUser.userName, PORT_NUM_SERVERM_TCP);

    verifyAuth(&newUser, sd_udp, address_ServerC);

    sendUserAuthFeedback(connected_sd_tcp); // send feedback to client via TCP
}

void connectServerC(struct sockaddr_in *server_address)
{
    /* connect to server */
    server_address->sin_family = AF_INET;
    server_address->sin_port = htons(PORT_NUM_SERVERC_UDP);
    server_address->sin_addr.s_addr = INADDR_ANY;
}

void commuClient(int *sd_tcp, int *sd_udp)
{
    /* TCP var */
    int connected_sd_tcp, connected_client_port;
    struct sockaddr_in client_address;
    socklen_t client_address_len;

    /* LOOP1 - TCP listen to incoming client, limit: one student */
    listen(*sd_tcp, 1);
    connected_sd_tcp = accept(*sd_tcp, (struct sockaddr *)&client_address, &client_address_len);

    struct sockaddr_in address_ServerC;
    connectServerC(&address_ServerC);

/* LOOP2 - receive message from connected clients */
CP_SESSION:
    authProcess(sd_tcp, &connected_sd_tcp, sd_udp, &address_ServerC);

    goto CP_SESSION;
}

int main(int argc, char *argv[])
{
    int sd_tcp, sd_udp; /* socket descriptor */

    initServerMTCP(&sd_tcp);
    initServerMUDP(&sd_udp);
    commuClient(&sd_tcp, &sd_udp);

    return 0;
}
