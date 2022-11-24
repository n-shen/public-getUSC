#include "header.h"

/*
 * Function: initClient
 * ----------------------------
 *   Create client socket and bind with its IP addr
 *
 *   *sd: client socket descriptor
 *
 *   returns: the dynamic port number assigned to client
 */
int initClient(int *sd)
{
    /* initialize client socket descriptor */
    *sd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in my_address; /* initialize client IP address */
    socklen_t my_address_len = sizeof(my_address);
    my_address.sin_family = AF_INET;
    my_address.sin_addr.s_addr = inet_addr(IP_CLIENT);
    my_address.sin_port = 0; /* assign client port dynamically */

    /* bind client socket and ip */
    if (bind(*sd, (struct sockaddr *)&my_address, my_address_len) < 0)
    {
        perror("[ERROR] Client-binding error");
        exit(-1);
    }
    printf("The client is up and running.\n"); /* on-screen message */

    /* get client port number */
    if (getsockname(*sd, (struct sockaddr *)&my_address, &my_address_len) == -1)
    {
        perror("[Error] Client-getsocket name");
        exit(-1);
    }

    return ntohs(my_address.sin_port); /* return client dynamic port number */
}

/*
 * Function: connServerM
 * ----------------------------
 *   Connect with ServerM via TCP
 *
 *   *sd: client socket descriptor
 */
void connServerM(int *sd)
{
    /* create a socket descriptor connecting to ServerM */
    struct sockaddr_in serverM_address;
    serverM_address.sin_family = AF_INET;
    serverM_address.sin_addr.s_addr = inet_addr(IP_SERVERM);
    serverM_address.sin_port = htons(PORT_NUM_SERVERM_TCP);

    /* connect to serverM */
    if (connect(*sd, (struct sockaddr *)&serverM_address, sizeof(struct sockaddr_in)) < 0)
    {
        close(*sd);
        perror("[Error] Client-connecting socket with ServerM");
        exit(-1);
    }
}

/*
 * Function: askUserAuth
 * ----------------------------
 *   Ask user for authentication input
 *
 *   *userAuth: user authentication profile field (e.x. userName or userPsw)
 *   type: 0-username, 1-password
 */
void askUserAuth(char *userAuth, int type)
{
    char buffer[BUFFSIZE];
    (type) ? (printf("Please enter the password: ")) : (printf("Please enter the username: "));
    fflush(stdout);
    fgets(buffer, sizeof(buffer), stdin); /* retrieve user input from console */
    buffer[strcspn(buffer, "\n")] = 0;
    strcpy(userAuth, buffer);
}

/*
 * Function: sendUserAuth
 * ----------------------------
 *   Send user authentication request to ServerM
 *
 *   *sd: client socket descriptor
 *   *newUser: user Auth profile
 */
void sendUserAuth(int *sd, struct User_auth *newUser)
{
    askUserAuth(newUser->userName, 0); /* ask user for userName */
    askUserAuth(newUser->userPsw, 1);  /* ask user for userPsw */

    /* Send user authentication to the serverM via TCP */
    if (write(*sd, (struct User_auth *)newUser, sizeof(struct User_auth)) < 0)
        perror("AuthReq send failed");
    printf("%s sent an authentication request to the main server.\n", newUser->userName); /* on-screen message */
}

/*
 * Function: recvUserAuthFeedback
 * ----------------------------
 *   Receive user authentication result from ServerM
 *   Send feedback code to caller function "commuServerM()"
 *
 *   sd: client socket descriptor
 *   my_port_num: the dynamic port number assigned to client
 *   *userName: user Auth profile -> name
 *   *authAttempts: the remaining number of login attempts
 *
 */
void recvUserAuthFeedback(int sd, int my_port_num, char *userName, int *authAttempts)
{
    char authFeedback[FEEDBACKSIZE];
    /* receive the auth feedback from serverM */
    if (read(sd, &authFeedback, sizeof(authFeedback)) <= 0)
    {
        printf("Auth Feedback received failed, try to connect server later.\n");
        exit(-1);
    }
    printf("%s received the result of authentication using TCP over port %d. ", userName, my_port_num);

    switch (atoi(authFeedback))
    {
    case 101:
        *authAttempts -= 1;
        printf("Authentication failed: Username does not exist\n");
        printf("Attempts remaining:%d\n", *authAttempts);
        break;

    case 102:
        *authAttempts -= 1;
        printf("Authentication failed: Password does not match\n");
        printf("Attempts remaining:%d\n", *authAttempts);
        break;

    case 103:
        *authAttempts = -99; /* set "-99" as authentication is successful */
        printf("Authentication is successful\n");
        break;

    default:
        break;
    }
}

/*
 * Function: askUserQuery
 * ----------------------------
 *   Ask user for query input
 *
 *   *userQuery: user query request field (e.x. course code or category)
 *   type: 1-course, 0-category
 */
void askUserQuery(char *userQuery, int type)
{
    char buffer[BUFFSIZE];
    (type) ? (printf("Please enter the course code to query: ")) : (printf("Please enter the category (Credit/Professor/Days/CourseName): "));
    fflush(stdout);
    fgets(buffer, sizeof(buffer), stdin); /* retrieve user input from console */
    buffer[strcspn(buffer, "\n")] = 0;
    strcpy(userQuery, buffer);
}

/*
 * Function: userQuery
 * ----------------------------
 *   Process user query request with ServerM
 *
 *   *sd: client socket descriptor
 *   *userName: user name
 */
void userQuery(int *sd, char *userName)
{
    struct User_query newQuery;
    askUserQuery(newQuery.course, 1);   /* ask user for coursecode */
    askUserQuery(newQuery.category, 0); /* ask user for category */

    for (int i = 0; newQuery.category[i]; i++)
        newQuery.category[i] = tolower(newQuery.category[i]);
    for (int i = 0; newQuery.course[i]; i++)
        newQuery.course[i] = toupper(newQuery.course[i]);

    /* Send user query to the serverM via TCP */
    if (write(*sd, (struct User_query *)&newQuery, sizeof(struct User_query)) < 0)
        perror("QueryRequest send failed");
    printf("%s sent a request to the main server.\n", userName); /* on-screen message */
    printf("The %s of %s is XXX.\n", newQuery.category, newQuery.course);
}

/*
 * Function: commuServerM
 * ----------------------------
 *  Client main chanel with ServerM
 *  Communicate with ServerM including serveral important sessions:
 *  e.x. AUTH_SESSION and MAIN_SESSION
 *
 *  *sd: client socket descriptor
 *  my_port_num: the dynamic port number assigned to client
 */
void commuServerM(int *sd, int my_port_num)
{
    int authAttempts = AUTHATTEMPTS; /* maximum login attempts, must greater than 0 */
    struct User_auth newUser;        /* a User Auth profile */

AUTH_SESSION:                   /* AUTH_SESSION: process authentication request with serverM */
    sendUserAuth(sd, &newUser); /* send authentication request to serverM */
    recvUserAuthFeedback(*sd, my_port_num, newUser.userName, &authAttempts);
    if (authAttempts == 0) /* used up all attempts */
    {
        printf("Authentication Failed for %d attempts. Client will shut down.\n", AUTHATTEMPTS);
        close(*sd); /* close current client socket */
        exit(-1);   /* terminate current AUTH_SESSION and exit client */
    }
    else if (authAttempts == -99) /* if auth is successful, goto MAIN_SESSION */
        goto MAIN_SESSION;
    goto AUTH_SESSION; /* if auth is NOT successful, repeat AUTH_SESSION */

MAIN_SESSION:                        /* MAIN_SESSION: major query tasks */
    userQuery(sd, newUser.userName); /* process user query */
    printf("\n-----Start a new request-----\n");
    goto MAIN_SESSION;
}

/*
 * Function: main
 * ----------------------------
 *  Client entry point
 *  Initialize client and communicate with ServerM
 */
int main(int argc, char *argv[])
{
    int sd, my_port_num;            /* client socket descriptor and client port number */
    my_port_num = initClient(&sd);  /* initialize client TCP portal and get dynamic port number */
    connServerM(&sd);               /* connect with serverM via TCP */
    commuServerM(&sd, my_port_num); /* communicate with serverM via TCP */

    return 0;
}
