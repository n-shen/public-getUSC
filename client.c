#include "header.h"

/* get client port number */
int getMyPortNum(int sd, struct sockaddr_in my_address, socklen_t my_address_len)
{

    if (getsockname(sd, (struct sockaddr *)&my_address, &my_address_len) == -1)
    {
        perror("[Error] getsocket name");
        exit(-1);
    }

    return ntohs(my_address.sin_port);
}

int initClient(int *sd)
{
    struct sockaddr_in my_address;

    /* create a client socket */
    *sd = socket(AF_INET, SOCK_STREAM, 0);
    my_address.sin_family = AF_INET;
    my_address.sin_addr.s_addr = INADDR_ANY;
    my_address.sin_port = 0; // assign client port dynamically

    /* client: bind socket and ip */
    if (bind(*sd, (struct sockaddr *)&my_address, sizeof(my_address)) < 0)
    {
        perror("[Error] client binding error");
        exit(-1);
    }

    /* on screen */
    printf("The client is up and running.\n");

    /* return client port number to main */
    return getMyPortNum(*sd, my_address, sizeof(my_address));
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

int main(int argc, char *argv[])
{
    int sd, my_port_num;
    struct sockaddr_in my_address;

    my_port_num = initClient(&sd);
    connServerM(&sd);
    commuServerM(&sd, my_port_num);

    return 0;
}
