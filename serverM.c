#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFSIZE 100

int main(int argc, char *argv[])
{
    int sd;           /* socket descriptor */
    int connected_sd; /* socket descriptor */
    int rc;           /* return code from recvfrom */
    struct sockaddr_in server_address;
    struct sockaddr_in from_address;
    socklen_t fromLength;
    int portNumber;
    int formLength;

    /* usage reminder */
    if (argc < 2)
    {
        printf("Usage is: server <portNumber>\n");
        exit(1);
    }

    /* create a socket */
    portNumber = atoi(argv[1]);
    sd = socket(AF_INET, SOCK_STREAM, 0);

    formLength = sizeof(struct sockaddr_in);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portNumber);
    server_address.sin_addr.s_addr = INADDR_ANY;

    /* bind socket and ip */
    rc = bind(sd, (struct sockaddr *)&server_address, sizeof(server_address));
    if (rc < 0)
    {
        perror("bind");
        exit(1);
    }
    else
    {
        printf("/*-------- Server Status: Established! -----------*/\n");
    }

/* LOOP1 - listen to incoming clients */
LOOP1:
    listen(sd, 5);
    connected_sd = accept(sd, (struct sockaddr *)&from_address, &fromLength);

/* LOOP2 - receive message from connected clients */
LOOP2:
    printf("\n[SERVER Notice] Clinet conneted! Listening...\n");

    /* Server - Read file name and filename size*/
    int sizeOfFilename;
    char buffer[100];
    memset(buffer, 0, 100);
    char *ptr = buffer;

    /* read size */
    rc = read(connected_sd, &sizeOfFilename, sizeof(int));
    if (rc <= 0)
    {
        printf("\n\n/*-------- Client Status: Clinet disconneted! Waiting new clients... -----------*/\n");
        goto LOOP1;
    }

    sizeOfFilename = ntohs(sizeOfFilename);
    /* read message */
    rc = read(connected_sd, ptr, sizeOfFilename);
    if (rc <= 0)
    {
        perror("reveive error");
        exit(1);
    }

    /* Server - Print result */
    char originName[100];
    strncpy(originName, buffer, 100);
    printf("[SERVER Notice] Received string: \"%s\"\n", originName);

    goto LOOP2;

    return 0;
}