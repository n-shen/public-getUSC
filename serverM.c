#include "header.h"

void commuClient(struct ServerM *serverM_API);

/*
 * Function: initServerMUDP
 * ----------------------------
 *   Create ServerM UDP socket and bind with its IP addr IP_SERVERM
 *
 *   *sd_udp: serverM socket descriptor
 *
 */
void initServerMUDP(int *sd_udp)
{
    struct sockaddr_in serverM_UDP_address;
    /* Create serverM socket. */
    *sd_udp = socket(AF_INET, SOCK_DGRAM, 0);

    serverM_UDP_address.sin_family = AF_INET;
    serverM_UDP_address.sin_port = htons(PORT_NUM_SERVERM_UDP);
    serverM_UDP_address.sin_addr.s_addr = INADDR_ANY;

    /* bind and check return code from binding */
    if (bind(*sd_udp, (struct sockaddr *)&serverM_UDP_address, sizeof(serverM_UDP_address)) < 0)
    {
        perror("serverM Warning: UDP bind error");
        exit(-1);
    }
}

/*
 * Function: initServerMTCP
 * ----------------------------
 *   Create ServerM TCP socket and bind with its IP addr IP_SERVERM
 *
 *   *sd_tcp: serverM socket descriptor
 */
void initServerMTCP(int *sd_tcp)
{
    struct sockaddr_in serverM_address;

    /* Create serverM socket. */
    *sd_tcp = socket(AF_INET, SOCK_STREAM, 0);

    serverM_address.sin_family = AF_INET;
    serverM_address.sin_port = htons(PORT_NUM_SERVERM_TCP);
    serverM_address.sin_addr.s_addr = INADDR_ANY;

    /* Bind and check return code from binding */
    if (bind(*sd_tcp, (struct sockaddr *)&serverM_address, sizeof(serverM_address)) < 0)
    {
        perror("serverM Warning: bind error");
        exit(-1);
    }

    printf("The main server is up and running.\n");
}

/*
 * Function: bindServerC
 * ----------------------------
 *   Bind with serverC via UDP
 *
 *   *server_address: serverC address
 */
void bindServerC(struct sockaddr_in *server_address)
{
    /* bind with serverC */
    server_address->sin_family = AF_INET;
    server_address->sin_port = htons(PORT_NUM_SERVERC_UDP);
    server_address->sin_addr.s_addr = INADDR_ANY;
}

/*
 * Function: bindServerEE
 * ----------------------------
 *   Bind with serverC via UDP
 *
 *   *server_address: serverC address
 */
void bindServerEE(struct sockaddr_in *server_address)
{
    /* bind with serverEE */
    server_address->sin_family = AF_INET;
    server_address->sin_port = htons(PORT_NUM_SERVEREE_UDP);
    server_address->sin_addr.s_addr = INADDR_ANY;
}

/*
 * Function: recvUserAuth
 * ----------------------------
 *   Receive client's Auth request via TCP
 *
 *   *serverM_API: serverM API
 *   *userAuth: user auth profile
 */
void recvUserAuth(struct ServerM *serverM_API, struct User_auth *userAuth)
{
    int sizeOfUserAuth = sizeof(struct User_auth);
    struct User_auth *buffer = malloc(sizeOfUserAuth);

    if (read(serverM_API->connected_sd_tcp, buffer, ntohs(sizeOfUserAuth)) <= 0) /* read size and buffer */
    {
        printf("\n$------- Clinet disconneted! Waiting new clients... ----------$\n");
        commuClient(serverM_API); /* wait for new client */
    }
    memcpy(userAuth, buffer, sizeOfUserAuth); /* save result*/
}

/*
 * Function: encryptAuth
 * ----------------------------
 *   Encrypt client's Auth
 *
 *   *userAuth: user auth structure
 *
 */
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

/*
 * Function: verifyAuth
 * ----------------------------
 *   Verify client's Auth with ServerC via TCP
 *
 *   *serverM_API: serverM API
 *   *newUser: user auth structure
 *   *feedback: feedback code
 */
void verifyAuth(struct ServerM *serverM_API, struct User_auth *newUser, char *feedback)
{
    /* UDP: ServerC(my server) info init */
    int rc;
    socklen_t serverC_address_len;

    /* encrypt auth */
    encryptAuth(newUser->userName);
    encryptAuth(newUser->userPsw);
    printf("Encrypted: %s, %s.\n", newUser->userName, newUser->userPsw);
    if (sendto(serverM_API->sd_udp, (struct User_auth *)newUser, (1024 + sizeof(newUser)), 0, (struct sockaddr *)&serverM_API->addr_ServerC, sizeof(serverM_API->addr_ServerC)) <= 0)
        perror("UDP send user auth req failed");
    printf("The main server sent an authentication request to serverC.\n");

    /* recv verification feedback from serverC */
    rc = recvfrom(serverM_API->sd_udp, (char *)feedback, FEEDBACKSIZE, MSG_WAITALL, (struct sockaddr *)&serverM_API->addr_ServerC, &serverC_address_len);
    if (rc <= 0)
        perror("ServerM recv feedback failed");
    feedback[rc] = '\0';
    printf("The main server received the result of the authentication request from ServerC using UDP over port %d.\n", PORT_NUM_SERVERM_UDP);
}

/*
 * Function: authProcess
 * ----------------------------
 *   Auth: Receive, verify through serverC, and send feedback to client.
 *
 *   *serverM_API: serverM API
 *   *userName: user name
 *
 *   Returns feedback code
 */
int authProcess(struct ServerM *serverM_API, char *userName)
{
    struct User_auth newUser;
    char fbCode[FEEDBACKSIZE]; /* feedback code */

    recvUserAuth(serverM_API, &newUser); /* receive user Auth request from client */
    strncpy(userName, newUser.userName, BUFFSIZE);
    printf("Received Auth: [%s,%s]\n", newUser.userName, newUser.userPsw);
    printf("The main server received the authentication for %s using TCP over port %d.\n", newUser.userName, PORT_NUM_SERVERM_TCP);

    verifyAuth(serverM_API, &newUser, fbCode);                            /* verify auth via serverC */
    if (write(serverM_API->connected_sd_tcp, fbCode, sizeof(fbCode)) < 0) /* send feedback to client via TCP */
        perror("User Auth Feedback sent failed");
    printf("The main server sent the authentication result to client. fbcode: %s\n", fbCode);

    return atoi(fbCode);
}

/*
 * Function: recvUserQuery
 * ----------------------------
 *   Query: Receive query request from client.
 *
 *   *serverM_API: serverM API
 *   *userQuery: user query request
 *
 */
void recvUserQuery(struct ServerM *serverM_API, struct User_query *userQuery)
{
    int sizeOfUserQuery = sizeof(struct User_query);
    struct User_query *buffer = malloc(sizeOfUserQuery);

    if (read(serverM_API->connected_sd_tcp, buffer, ntohs(sizeOfUserQuery)) <= 0) /* read size and buffer */
    {
        printf("\n$------- Clinet disconneted! Waiting new clients... ----------$\n");
        commuClient(serverM_API); /* wait for new client */
    }
    memcpy(userQuery, buffer, sizeOfUserQuery); /* save result*/
}

/*
 * Function: retrieveCourse
 * ----------------------------
 *   Verify client's Auth with ServerC via TCP
 *
 *   *serverM_API: serverM API
 *   *newUser: user auth structure
 *   *feedback: feedback code
 */
void retrieveCourse(struct ServerM *serverM_API, struct User_query *query, char *result)
{
    /* UDP: serverEE and serverCS info init */
    int rc;
    socklen_t serverEE_address_len;
    socklen_t serverCS_address_len;

    /* encrypt auth */
    if (strncmp(query->course, "EE", 2) == 0)
    {
        if (sendto(serverM_API->sd_udp, (struct User_query *)query, (1024 + sizeof(query)), 0, (struct sockaddr *)&serverM_API->addr_ServerEE, sizeof(serverM_API->addr_ServerEE)) <= 0)
            perror("UDP send user query request failed");
        printf("The main server sent a request to serverEE.\n");
        /* recv verification feedback from serverEE */
        rc = recvfrom(serverM_API->sd_udp, (char *)result, QUERYRESULTSIZE, MSG_WAITALL, (struct sockaddr *)&serverM_API->addr_ServerEE, &serverEE_address_len);
        if (rc <= 0)
            perror("ServerM recv feedback failed");
        result[rc] = '\0';
        printf("The main server received the response from ServerEE using UDP over port %d.\n", PORT_NUM_SERVERM_UDP);
    }
}

/*
 * Function: queryProcess
 * ----------------------------
 *   Query: Receive, retrieve through serverEE/severCS, and send result to client.
 *
 *   *serverM_API: serverM API
 *   *userName: user name
 */
void queryProcess(struct ServerM *serverM_API, char *userName)
{
    struct User_query newQuery;
    char result[QUERYRESULTSIZE]; /* query result */

    recvUserQuery(serverM_API, &newQuery); /* receive user query request from client */
    printf("The main server received from %s to query course %s about %s using TCP over port %d.\n", userName, newQuery.course, newQuery.category, PORT_NUM_SERVERM_TCP);
    retrieveCourse(serverM_API, &newQuery, result);

    if (write(serverM_API->connected_sd_tcp, result, sizeof(result)) < 0) /* send query result to client via TCP */
        perror("User query result sent failed");
    printf("The main server sent the query information to the client.\n");
}

/*
 * Function: commuClient
 * ----------------------------
 *   Communicate with client, one client per session
 *
 *   *serverM_API: serverM API
 */
void commuClient(struct ServerM *serverM_API)
{
    socklen_t address_client_len;
    char userName[BUFFSIZE];

    listen(serverM_API->sd_tcp, 1);                                                                                                      /* TCP listen to incoming client, limit to one student per session */
    serverM_API->connected_sd_tcp = accept(serverM_API->sd_tcp, (struct sockaddr *)&serverM_API->connected_sd_tcp, &address_client_len); /* accept to client's request */

AUTH_SESSION:
    if (authProcess(serverM_API, userName) == 103) /* LOOP - receive message from connected clients */
        goto MAIN_SESSION;
    else
        goto AUTH_SESSION;

MAIN_SESSION:
    queryProcess(serverM_API, userName);
    goto MAIN_SESSION;
}

int main(int argc, char *argv[])
{
    struct ServerM serverM_API;

    initServerMTCP(&serverM_API.sd_tcp); /* initialize server */
    initServerMUDP(&serverM_API.sd_udp);
    bindServerC(&serverM_API.addr_ServerC);   /* bind with serverC */
    bindServerEE(&serverM_API.addr_ServerEE); /* bind with serverEE */
    commuClient(&serverM_API);                /* communicate with client */

    return 0;
}
