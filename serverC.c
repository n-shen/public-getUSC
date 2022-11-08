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

int validateAuth(struct User_auth auth)
{
    /* file sys */
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    char *name, *psw;
    int code = 101;

    fp = fopen("./cred.txt", "r");
    if (fp == NULL)
    {
        printf("[ERROR] cred.txt load failed.\n");
        exit(EXIT_FAILURE);
    }

    while ((read = getline(&line, &len, fp)) != -1)
    {
        name = strtok(line, ",");
        if (strcmp(name, auth.userName) == 0)
        {
            code += 1;
            psw = strtok(NULL, ",");
            if (strcmp(psw, auth.userPsw) == 0)
                code += 1;
            break;
        }
    }

    fclose(fp); /* close file */

    return code;
}

void commuServerM(int *sd)
{
    /* ServerM(my client) info init */
    int rc;
    struct sockaddr_in serverM_address;
    socklen_t serverM_address_len;
    struct User_auth *buffer = malloc(sizeof(struct User_auth));
    char feedback[FEEDBACKSIZE];

LOOP1:
    /* ServerM(my client) recv */
    rc = recvfrom(*sd, (struct User_auth *)buffer, (sizeof(*buffer)), MSG_WAITALL, (struct sockaddr *)&serverM_address, &serverM_address_len);
    if (rc <= 0)
        perror("ServerC recv req failed");
    printf("The ServerC received an authentication request from the Main Server.\n");
    /* Server - create upload report */
    printf("[Server Notice] From ServerM, username: %s, psw:%s.\n", buffer->userName, buffer->userPsw);

    sprintf(feedback, "%d", validateAuth(*buffer));

    rc = sendto(*sd, (char *)feedback, FEEDBACKSIZE, 0, (struct sockaddr *)&serverM_address, sizeof(serverM_address));
    if (rc <= 0)
        perror("ServerC send feedback failed");

    printf("The ServerC finished sending the response to the Main Server.\n");

    goto LOOP1; /* repeat */
}

int main(int argc, char *argv[])
{
    int sd; /* socket descriptor */

    initServerC(&sd);
    commuServerM(&sd);

    return 0;
}
