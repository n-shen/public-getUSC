#include "header.h"

/*
 * Function: initServerC
 * ----------------------------
 *   Create ServerC UDP socket and bind with its IP addr
 *
 *   *sd: serverM UDP socket descriptor
 *
 */
void initServerC(int *sd)
{
    struct sockaddr_in serverC_address;
    *sd = socket(AF_INET, SOCK_DGRAM, 0); /* Create serverM socket. */

    serverC_address.sin_family = AF_INET;
    serverC_address.sin_port = htons(PORT_NUM_SERVERC_UDP);
    serverC_address.sin_addr.s_addr = INADDR_ANY;

    /* Bind ServerC socket and address. */
    if (bind(*sd, (struct sockaddr *)&serverC_address, sizeof(serverC_address)) < 0)
    {
        perror("[ERROR] serverC bind error");
        exit(-1);
    }

    printf("The ServerC is up and running using UDP on port %d.\n", PORT_NUM_SERVERC_UDP);
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
        exit(-1);
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
    struct User_auth buffer;
    char feedback[FEEDBACKSIZE];

SESSION:
    /* receive request from ServerM(my client) */
    rc = recvfrom(*sd, (struct User_auth *)&buffer, (sizeof(buffer)), MSG_WAITALL, (struct sockaddr *)&serverM_address, &serverM_address_len);
    if (rc <= 0)
        perror("ServerC recv req failed");
    printf("The ServerC received an authentication request from the Main Server.\n");
    sprintf(feedback, "%d", validateAuth(buffer));

    /* send auth result back to serverM */
    rc = sendto(*sd, (char *)feedback, FEEDBACKSIZE, 0, (struct sockaddr *)&serverM_address, sizeof(serverM_address));
    if (rc <= 0)
        perror("[ERROR] ServerC send feedback failed");
    printf("The ServerC finished sending the response to the Main Server.\n");

    goto SESSION; /* repeat */
}

int main(int argc, char *argv[])
{
    int sd;            /* socket descriptor */
    initServerC(&sd);  /* initialize serverC */
    commuServerM(&sd); /* communicate with serverM */

    return 0;
}
