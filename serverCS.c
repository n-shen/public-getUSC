/* Author: Niholas Shen https://shen.dev
 * University of Southern California. EE450 Nov.28.2022
 */

#include "header.h"

/*
 * Function: initServerCS
 * ----------------------------
 *   Create ServerCS UDP socket and bind with its IP addr
 *
 *   *sd: serverM UDP socket descriptor
 */
void initServerCS(int *sd)
{
    struct sockaddr_in serverCS_address;
    *sd = socket(AF_INET, SOCK_DGRAM, 0); /* Create serverM socket. */
    memset(&serverCS_address, 0, sizeof(serverCS_address));
    serverCS_address.sin_family = AF_INET;
    serverCS_address.sin_port = htons(PORT_NUM_SERVERCS_UDP);
    serverCS_address.sin_addr.s_addr = inet_addr(IP_SERVERCS);

    /* Bind ServerCS socket and address. */
    if (bind(*sd, (struct sockaddr *)&serverCS_address, sizeof(serverCS_address)) < 0)
    {
        perror("[ERROR] serverCS bind error");
        exit(-1);
    }

    printf("The ServerCS is up and running using UDP on port %d.\n", PORT_NUM_SERVERCS_UDP); /* on-screen message */
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

    fp = fopen("./cs.txt", "r"); /* open credential db */
    if (fp == NULL)
    {
        printf("[ERROR] cs.txt load failed.\n");
        exit(-1);
    }

    /* look for course info */
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
 *   token: client query request
 *   *courseinfo: query result
 */
void retrieveMultiData(char *token, char *courseinfo)
{
    /* file sys */
    FILE *fp;
    char *code;
    char line[COURSEINFOSIZE];
    int found = -1;
    int i;

    fp = fopen("./cs.txt", "r"); /* open credential db */
    if (fp == NULL)
    {
        printf("[ERROR] cs.txt load failed.\n");
        exit(-1);
    }
    /* look for specific course info */
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

/*
 * Function: queryMutiProcess
 * ----------------------------
 *   Muti mode: process query
 *
 *   *courses: a list of course code
 *   *sd: client TCP socket
 *  *serverM_address API of serverM
 */
void queryMutiProcess(char *courses, int *sd, struct sockaddr_in *serverM_address)
{
    int i = 0, rc = 0;
    char courselist[MUTIQUERYSIZE][QUERYRESULTSIZE], courseinfos[MUTIQUERYSIZE][COURSEINFOSIZE]; /* muti courses' codes and infos */
    memset(courselist, 0, sizeof(courselist[0][0]) * 10 * QUERYRESULTSIZE);
    memset(courseinfos, 0, sizeof(courseinfos[0][0]) * 10 * COURSEINFOSIZE);

    /* read query request from serverM */
    char *token = strtok(courses, "&");
    while (token != NULL)
    {
        strcpy(courselist[i], token);
        token = strtok(NULL, "&");
        i += 1;
    }

    /* process each query */
    for (i = 0; i < 10; i++)
    {
        if (strlen(courselist[i]) == 0)
            break;
        retrieveMultiData(courselist[i], courseinfos[i]); /* retrieve course data and store into courseinfos */
    }

    /* send query result back to serverM */
    rc = sendto(*sd, (char *)courseinfos, 10 * COURSEINFOSIZE, 0, (struct sockaddr *)serverM_address, sizeof(*serverM_address));
    if (rc <= 0)
    {
        perror("[ERROR] ServerCS send feedback failed");
        exit(-1);
    }
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
    socklen_t serverM_address_len;
    struct sockaddr_in serverM_address;
    struct User_query buffer;
    char result[QUERYRESULTSIZE];
    serverM_address_len = sizeof(serverM_address);

SESSION:
    memset(&serverM_address, 0, sizeof(serverM_address));
    /* receive query request from ServerM */
    if (recvfrom(*sd, (struct User_query *)&buffer, (sizeof(buffer)), MSG_WAITALL, (struct sockaddr *)&serverM_address, &serverM_address_len) <= 0)
    {
        perror("[ERROR] ServerCS receiving request failed");
        exit(-1);
    }

    /* if in muti mode */
    if (strcmp("!muti!", buffer.category) == 0)
        queryMutiProcess(buffer.course, sd, &serverM_address);
    else
    {
        printf("The ServerCS received a request from the Main Server about %s of %s.\n", buffer.category, buffer.course); /* on-screen message */
        /* retrieve course info */
        retrieveData(buffer, result);
        /* send query result back to serverM */
        if (sendto(*sd, (char *)result, QUERYRESULTSIZE, 0, (struct sockaddr *)&serverM_address, serverM_address_len) <= 0)
        {
            perror("[ERROR] ServerCS send feedback failed");
            exit(-1);
        }
        printf("The ServerCS finished sending the response to the Main Server.\n"); /* on-screen message */
    }

    goto SESSION; /* repeat */
}

int main(int argc, char *argv[])
{
    int sd;            /* socket descriptor */
    initServerCS(&sd); /* initialize serverC */
    commuServerM(&sd); /* communicate with serverM */

    return 0;
}
