#include "header.h"

/*
 * Function: initServerEE
 * ----------------------------
 *   Create ServerEE UDP socket and bind with its IP addr
 *
 *   *sd: serverM UDP socket descriptor
 */
void initServerEE(int *sd)
{
    struct sockaddr_in serverEE_address;
    *sd = socket(AF_INET, SOCK_DGRAM, 0); /* Create serverM socket. */

    serverEE_address.sin_family = AF_INET;
    serverEE_address.sin_port = htons(PORT_NUM_SERVEREE_UDP);
    serverEE_address.sin_addr.s_addr = INADDR_ANY;

    /* Bind ServerEE socket and address. */
    if (bind(*sd, (struct sockaddr *)&serverEE_address, sizeof(serverEE_address)) < 0)
    {
        perror("[ERROR] serverEE bind error");
        exit(-1);
    }

    printf("The ServerEE is up and running using UDP on port %d.\n", PORT_NUM_SERVEREE_UDP); /* on-screen message */
}

/*
 * Function: retrieveData
 * ----------------------------
 *   retrieve course info from db
 *
 *   query: client query request
 *   *result: query result
 */
void retrieveData(struct User_query query, char *result)
{
    /* file sys */
    FILE *fp;
    char line[COURSEINFOSIZE];
    size_t len = 0;
    char *code, *credit, *professor, *days, *name;
    int found = -1;

    fp = fopen("./ee.txt", "r"); /* open credential db */
    if (fp == NULL)
    {
        printf("[ERROR] ee.txt load failed.\n");
        exit(-1);
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
                printf("The course information has been found: The credit of %s is %s.\n", query.course, result); /* on-screen message */
                break;
            }

            professor = strtok(NULL, ",");
            if (strcmp("professor", query.category) == 0)
            {
                strcpy(result, professor);
                printf("The course information has been found: The professor of %s is %s.\n", query.course, result); /* on-screen message */
                break;
            }

            days = strtok(NULL, ",");
            if (strcmp("days", query.category) == 0)
            {
                strcpy(result, days);
                printf("The course information has been found: The days of %s is %s.\n", query.course, result); /* on-screen message */
                break;
            }

            name = strtok(NULL, ",");
            if (strcmp("coursename", query.category) == 0)
            {
                if (name[strlen(name) - 1] == '\n')
                {
                    name[strcspn(name, "\n")] = 0;
                    name[strlen(name) - 1] = '\0';
                }
                strcpy(result, name);
                printf("The course information has been found: The coursename of %s is %s.\n", query.course, result); /* on-screen message */
                break;
            }
            found = 0;
        }
    }

    if (found == -1)
    {
        strcpy(result, "Didn't find the course!");
        printf("Didn't find the course: %s.\n", query.course); /* on-screen message */
    }

    if (found == 0)
    {
        strcpy(result, "Invalid Category!");
        printf("Invalid Category: %s.\n", query.category); /* on-screen message */
    }

    fclose(fp); /* close file */
}

/*
 * Function: retrieveMultiData
 * ----------------------------
 *   retrieve course lists info from db
 *
 *   query: client query request
 *   *multilist: query result
 */
void retrieveMultiData(char *token, char *courseinfo)
{
    /* file sys */
    FILE *fp;
    char *code;
    char line[COURSEINFOSIZE];
    int found = -1;
    int i;

    fp = fopen("./ee.txt", "r"); /* open credential db */
    if (fp == NULL)
    {
        printf("[ERROR] ee.txt load failed.\n");
        exit(-1);
    }
    /* check username and password */
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        code = strtok(line, ","); /* course code */
        if (strcmp(code, token) == 0)
        {
            found = 1;
            strcat(courseinfo, code);
            strcat(courseinfo, ": ");

            for (i = 0; i < 3; i++)
            {
                code = strtok(NULL, ","); /* credit, Professor, and days*/
                strcat(courseinfo, code);
                strcat(courseinfo, ", ");
            }

            code = strtok(NULL, ","); /* coursename */
            if (code[strlen(code) - 1] == '\n')
            {
                code[strcspn(code, "\n")] = 0;
                code[strlen(code) - 1] = '\0';
            }
            strcat(courseinfo, code);
            break;
        }
    }
    if (found == -1)
    {
        strcpy(courseinfo, "Didn't find the course: ");
        strcat(courseinfo, token);
    }

    fclose(fp); /* close file */
}

void queryMutiProcess(char *courses, int *sd, struct sockaddr_in *serverM_address)
{
    int i = 0, rc = 0;
    char courselist[10][QUERYRESULTSIZE], courseinfos[10][COURSEINFOSIZE];
    memset(courselist, 0, sizeof(courselist[0][0]) * 10 * QUERYRESULTSIZE);
    memset(courseinfos, 0, sizeof(courseinfos[0][0]) * 10 * COURSEINFOSIZE);

    char *token = strtok(courses, "&");
    while (token != NULL)
    {
        strcpy(courselist[i], token);
        token = strtok(NULL, "&");
        i += 1;
    }

    for (i = 0; i < 10; i++)
    {
        if (strlen(courselist[i]) == 0)
            break;
        retrieveMultiData(courselist[i], courseinfos[i]);
    }

    for (i = 0; i < 10; i++)
        printf("COURSEINFO: %s.\n", courseinfos[i]);

    /* send query result back to serverM */
    rc = sendto(*sd, (char *)courseinfos, 10 * COURSEINFOSIZE, 0, (struct sockaddr *)serverM_address, sizeof(*serverM_address));
    if (rc <= 0)
    {
        perror("[ERROR] ServerEE send feedback failed");
        exit(-1);
    }
    printf("The ServerEE finished sending the muti response to the Main Server.\n"); /* on-screen message */
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
    struct User_query buffer;
    char result[QUERYRESULTSIZE];

SESSION:
    /* receive query request from ServerM */
    rc = recvfrom(*sd, (struct User_query *)&buffer, (sizeof(buffer)), MSG_WAITALL, (struct sockaddr *)&serverM_address, &serverM_address_len);
    if (rc <= 0)
    {
        perror("[ERROR] ServerEE receiving request failed");
        exit(-1);
    }

    /* if in muti mode */
    if (strcmp("!muti!", buffer.category) == 0)
    {
        printf("The ServerEE get a muti request: %s.\n", buffer.course);
        queryMutiProcess(buffer.course, sd, &serverM_address);
    }
    else
    {
        printf("The ServerEE received a request from the Main Server about %s of %s.\n", buffer.category, buffer.course); /* on-screen message */

        /* retrieve course info */
        retrieveData(buffer, result);

        /* send query result back to serverM */
        rc = sendto(*sd, (char *)result, QUERYRESULTSIZE, 0, (struct sockaddr *)&serverM_address, sizeof(serverM_address));
        if (rc <= 0)
        {
            perror("[ERROR] ServerEE send feedback failed");
            exit(-1);
        }
        printf("The ServerEE finished sending the response to the Main Server.\n"); /* on-screen message */
    }

    goto SESSION; /* repeat */
}

int main(int argc, char *argv[])
{
    int sd;            /* socket descriptor */
    initServerEE(&sd); /* initialize serverC */
    commuServerM(&sd); /* communicate with serverM */

    return 0;
}
