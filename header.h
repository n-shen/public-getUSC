#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define BUFFSIZE 51
#define PORT_NUM_SERVERM_TCP 25448
#define PORT_NUM_SERVERM_UDP 24448
#define PORT_NUM_SERVERC_UDP 21448
#define IP_SERVERM "127.0.0.1"
#define IP_SERVERC "127.0.0.1"

struct User_auth
{
    char userName[BUFFSIZE];
    char userPsw[BUFFSIZE];
};
