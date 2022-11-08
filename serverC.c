#include "header.h"

void initServerC(int *sd)
{
    struct sockaddr_in serverC_address;
    /* Create serverM socket. */
    *sd = socket(AF_INET, SOCK_DGRAM, 0);

    serverC_address.sin_family = AF_INET;
    serverC_address.sin_port = htons(PORT_NUM_SERVERC_UDP);
    serverC_address.sin_addr.s_addr = INADDR_ANY;

    /* Bind ServerC socket and IP. */
    // check return code from bind
    if (bind(*sd, (struct sockaddr *)&serverC_address, sizeof(serverC_address)) < 0)
    {
        perror("serverC Warning: bind error");
        exit(-1);
    }

    printf("The ServerC is up and running using UDP on port %d.\n", PORT_NUM_SERVERC_UDP);
}

void commuServerM(int *sd)
{
    /* ServerM(my client) info init */
    int connected_sd;
    struct sockaddr_in serverM_address;
    socklen_t serverM_address_len;
    struct User_auth *buffer = malloc(sizeof(struct User_auth));

LOOP1:
    /* ServerM(my client) recv */
    connected_sd = recvfrom(*sd, (struct User_auth *)buffer, (sizeof(*buffer)), MSG_WAITALL, (struct sockaddr *)&serverM_address, &serverM_address_len);
    printf("rc: %d.\n", connected_sd);
    /* Server - create upload report */
    printf("[Server Notice] From ServerM, username: %s\n", buffer->userName);
    printf("[Server Notice] From ServerM, psw: %s\n", buffer->userPsw);

    goto LOOP1; /* repeat */
}

int main(int argc, char *argv[])
{
    int sd; /* socket descriptor */

    initServerC(&sd);
    commuServerM(&sd);

    return 0;
}
