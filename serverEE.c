#include "header.h"

/*
 * Function: initServerEE
 * ----------------------------
 *   Create ServerEE UDP socket and bind with its IP addr
 *
 *   *sd: serverM UDP socket descriptor
 *
 */
void initServerEE(int *sd)
{
    struct sockaddr_in serverEE_address;
    *sd = socket(AF_INET, SOCK_DGRAM, 0); /* Create serverM socket. */

    serverEE_address.sin_family = AF_INET;
    serverEE_address.sin_port = htons(PORT_NUM_SERVEREE_UDP);
    serverEE_address.sin_addr.s_addr = INADDR_ANY;

    /* Bind ServerC socket and address. */
    if (bind(*sd, (struct sockaddr *)&serverEE_address, sizeof(serverEE_address)) < 0)
    {
        perror("serverC Warning: bind error");
        exit(-1);
    }

    printf("The ServerEE is up and running using UDP on port %d.\n", PORT_NUM_SERVEREE_UDP);
}

/*
 * Function: validateAuth
 * ----------------------------
 *   validate auth request
 *
 *   auth: server auth structure
 */
int validateAuth(struct User_auth auth)
{
    /* file sys */
    FILE *fp;
    char line[BUFFSIZE * 2 + 1];
    size_t len = 0;
    ssize_t read;

    int code = 101; /* feedback code */
    char *name, *psw;

    fp = fopen("./cred.txt", "r"); /* open credential db */
    if (fp == NULL)
    {
        printf("[ERROR] cred.txt load failed.\n");
        exit(EXIT_FAILURE);
    }

    /* check username and password */
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        name = strtok(line, ",");
        if (strcmp(name, auth.userName) == 0)
        {
            code += 1;
            psw = strtok(NULL, ",");
            if (psw[strlen(psw) - 1] == '\n')
            {
                psw[strcspn(psw, "\n")] = 0;
                psw[strlen(psw) - 1] = '\0';
            }

            if (strcmp(psw, auth.userPsw) == 0)
                code += 1;
            printf("psw passed: %s.\n", psw);
            break;
        }
    }

    fclose(fp); /* close file */

    return code;
}

/*
 * Function: commuServerM
 * ----------------------------
 *   Communicate with serverM
 *
 *   *sd: serverM socket descriptor for UDP
 */
void commuServerM(int *sd)
{
    /* ServerM(my client) info init */
    int rc;
    struct sockaddr_in serverM_address;
    socklen_t serverM_address_len;
    struct User_query *buffer = malloc(sizeof(struct User_query));
    char result[QUERYRESULTSIZE];

SESSION:
    /* receive query request from ServerM */
    rc = recvfrom(*sd, (struct User_auth *)buffer, (sizeof(*buffer)), MSG_WAITALL, (struct sockaddr *)&serverM_address, &serverM_address_len);
    if (rc <= 0)
    {
        perror("ServerEE recv req failed");
    }
    printf("The ServerEE received a request from the Main Server about %s of %s.\n", buffer->category, buffer->course);

    /* retrieve course info */
    strncpy(result, "Didn't find the course.", QUERYRESULTSIZE);

    /* send query result back to serverM */
    rc = sendto(*sd, (char *)result, QUERYRESULTSIZE, 0, (struct sockaddr *)&serverM_address, sizeof(serverM_address));
    if (rc <= 0)
        perror("ServerEE send feedback failed");

    printf("The ServerEE finished sending the response to the Main Server.\n");

    goto SESSION; /* repeat */
}

int main(int argc, char *argv[])
{
    int sd; /* socket descriptor */

    initServerEE(&sd); /* initialize serverC */
    commuServerM(&sd); /* communicate with serverM */

    return 0;
}
