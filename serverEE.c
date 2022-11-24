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
 * Function: retrieveData
 * ----------------------------
 *   validate auth request
 *
 *   auth: server auth structure
 */
void retrieveData(struct User_query query, char *result)
{
    /* file sys */
    FILE *fp;
    char line[COURSEINFOSIZE];
    size_t len = 0;
    char *code, *credit, *professor, *days, *name;
    int found = 0;

    fp = fopen("./ee.txt", "r"); /* open credential db */
    if (fp == NULL)
    {
        printf("[ERROR] ee.txt load failed.\n");
        exit(EXIT_FAILURE);
    }

    /* check username and password */
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        code = strtok(line, ",");
        if (strcmp(code, query.course) == 0)
        {
            found = 1;
            credit = strtok(NULL, ",");
            if (strcmp("credit", query.category) == 0)
            {
                strcpy(result, credit);
                break;
            }

            professor = strtok(NULL, ",");
            if (strcmp("professor", query.category) == 0)
            {
                strcpy(result, professor);
                break;
            }
        }
    }

    if (!found)
        strcpy(result, "Didn't find the course.");

    fclose(fp); /* close file */
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
    retrieveData(*buffer, result);

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
