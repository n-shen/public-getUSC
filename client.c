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
    *sd = socket(AF_INET, SOCK_STREAM, 0); /* initialize client socket descriptor */

    struct sockaddr_in my_address; /* initialize client IP address */
    socklen_t my_address_len = sizeof(my_address);
    my_address.sin_family = AF_INET;
    my_address.sin_addr.s_addr = inet_addr(IP_CLIENT);
    my_address.sin_port = 0; /* assign client port dynamically */

    /* bind client socket and ip */
    if (bind(*sd, (struct sockaddr *)&my_address, my_address_len) < 0)
    {
        perror("[ERROR] client-binding error");
        exit(-1);
    }
    printf("The client is up and running.\n"); /* on-screen message */

    /* get client port number */
    if (getsockname(*sd, (struct sockaddr *)&my_address, &my_address_len) == -1)
    {
        perror("[Error] client-getsocket name");
        exit(-1);
    }

    return ntohs(my_address.sin_port); /* return client dynamic port number */
}

void connServerM(int *sd)
{
    struct sockaddr_in serverM_address;

    /* create a ServerM socket */
    serverM_address.sin_family = AF_INET;
    serverM_address.sin_addr.s_addr = inet_addr(IP_SERVERM);
    serverM_address.sin_port = htons(PORT_NUM_SERVERM_TCP);

    /* connect to serverM */
    if (connect(*sd, (struct sockaddr *)&serverM_address, sizeof(struct sockaddr_in)) < 0)
    {
        close(*sd);
        perror("[Error] Client to ServerM - connecting stream socket");
        exit(-1);
    }
}

/* Ask client for Auth input */
void askUserAuth(char *userAuth, int type)
{
    char buffer[BUFFSIZE];
    (type) ? (printf("Please enter the password: ")) : (printf("Please enter the username: "));
    fflush(stdout);
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    strcpy(userAuth, buffer);
    // printf("input len:%lu.", strlen(userAuth));
}

void sendUserAuth(int *sd, struct User_auth *newUser)
{

    /* Ask client for Auth input */
    askUserAuth(newUser->userName, 0); // 0: username;
    askUserAuth(newUser->userPsw, 1);  // 1: password;
    printf("%s, %s\n", newUser->userName, newUser->userPsw);

    /* Send message(string) to the serverM via TCP */
    if (write(*sd, (struct User_auth *)newUser, sizeof(struct User_auth)) < 0)
        perror("AuthReq send failed");

    printf("%s sent an authentication request to the main server.\n", newUser->userName);
}

void recvUserAuthFeedback(int sd, int my_port_num, char *userName, int *authAttempts)
{
    char authFeedback[FEEDBACKSIZE];
    if (read(sd, &authFeedback, sizeof(authFeedback)) <= 0)
        perror("Auth Feedback received failed");
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
        *authAttempts = -99;
        printf("Authentication is successful\n");
        break;

    default:
        break;
    }
}

void commuServerM(int *sd, int my_port_num)
{
    int authAttempts = AUTHATTEMPTS;
    struct User_auth newUser;

    /* AUTH_SESSION: send auth msg to server repeatly */
AUTH_SESSION:
    sendUserAuth(sd, &newUser);
    recvUserAuthFeedback(*sd, my_port_num, newUser.userName, &authAttempts);
    if (authAttempts == 0)
    {
        printf("Authentication Failed for %d attempts. Client will shut down.\n", AUTHATTEMPTS);
        exit(-1);
    }
    else if (authAttempts == -99)
        goto MAIN_SESSION;
    goto AUTH_SESSION;

MAIN_SESSION:
    printf("Logged In.\n");
}

/*
 * Function: Main
 * ----------------------------
 *  Client entry point
 *  Initialize client and communicate with ServerM
 */
int main(int argc, char *argv[])
{
    int sd, my_port_num;            /* client socket descriptor and client port number */
    my_port_num = initClient(&sd);  /* initialize client TCP portal and get dynamic port number */
    connServerM(&sd);               /* connect with serverM via TCP */
    commuServerM(&sd, my_port_num); /* connect with serverM via TCP */

    return 0;
}
