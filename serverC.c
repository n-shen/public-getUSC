#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define BUFFSIZE 51
#define PORT_NUM_SERVERC 21448
#define IP_SERVERM "127.0.0.1"

void initServerC(int *sd)
{
    struct sockaddr_in serverC_address;
    /* Create serverM socket. */
    *sd = socket(AF_INET, SOCK_DGRAM, 0);

    serverC_address.sin_family = AF_INET;
    serverC_address.sin_port = htons(PORT_NUM_SERVERC);
    serverC_address.sin_addr.s_addr = INADDR_ANY;

    /* Bind ServerC socket and IP. */
    // check return code from bind
    if (bind(*sd, (struct sockaddr *)&serverC_address, sizeof(serverC_address)) < 0)
    {
        perror("serverC Warning: bind error");
        exit(-1);
    }

    printf("The ServerC is up and running using UDP on port %d.\n", PORT_NUM_SERVERC);
}

void commuServerM(int *sd)
{
    int connected_sd; /* return code from recvfrom */
    struct sockaddr_in serverM_address;
    socklen_t serverM_address_len;
    char buffer[BUFFSIZE];

LOOP1:
    connected_sd = recvfrom(*sd, (char *)buffer, BUFFSIZE, MSG_WAITALL, (struct sockaddr *)&serverM_address, &serverM_address_len);
    printf("rc: %d.\n", connected_sd);
    /* Server - create upload report */
    buffer[connected_sd] = '\0';
    printf("[Server Notice] From Server: %s\n", buffer);

    goto LOOP1; /* repeat */
}

int main(int argc, char *argv[])
{
    int sd; /* socket descriptor */

    initServerC(&sd);
    commuServerM(&sd);

    return 0;
}
