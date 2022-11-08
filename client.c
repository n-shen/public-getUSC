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

void recvUserAuthFeedback(int sd, int *authAttempts)
{
    int authFeedback = -1;
    if (read(sd, &authFeedback, sizeof(int)) <= 0)
        perror("Auth Feedback received failed");

    switch (ntohs(authFeedback))
    {
    case 101:
        *authAttempts -= 1;
        printf("Auth fb: %d.\n", ntohs(authFeedback));
        break;

    default:
        break;
    }
}

void commuServerM(int *sd)
{
    int authAttempts = 3;
    struct User_auth newUser;

    /* CP_SESSION: send message to server repeatly */
CP_SESSION:

    sendUserAuth(sd, &newUser);
    recvUserAuthFeedback(*sd, &authAttempts);

    if (authAttempts == 0)
        exit(-1);

    goto CP_SESSION;
}

int main(int argc, char *argv[])
{
    int sd, my_port_num;
    struct sockaddr_in my_address;

    my_port_num = initClient(&sd);
    printf("Client port: %d.\n", my_port_num);
    connServerM(&sd);
    commuServerM(&sd);

    return 0;
}
