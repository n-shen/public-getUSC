#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define BUFFSIZE 51
#define FEEDBACKSIZE 4
#define QUERYRESULTSIZE 1024
#define AUTHATTEMPTS 3 /* maximum login attempts, must greater than 0 */

#define IP_CLIENT "127.0.0.1"  /* client IP address */
#define IP_SERVERM "127.0.0.1" /* serverM IP address */
#define IP_SERVERC "127.0.0.1" /* serverM IP address */

#define PORT_NUM_SERVERM_TCP 25448  /* serverM TCP port num */
#define PORT_NUM_SERVERM_UDP 24448  /* serverM UDP port num */
#define PORT_NUM_SERVERC_UDP 21448  /* serverC UDP port num */
#define PORT_NUM_SERVEREE_UDP 23448 /* serverEE UDP port num */

/* DataStructure: ServerM_API */
struct ServerM
{
    int sd_tcp; /* TCP socket descriptor */
    int sd_udp; /* UDP socket descriptor */
    int connected_sd_tcp;
    struct sockaddr_in addr_ServerC;
    struct sockaddr_in addr_ServerEE;
    struct sockaddr_in addr_ServerCS;
};

/* DataStructure: User Auth file */
struct User_auth
{
    char userName[BUFFSIZE];
    char userPsw[BUFFSIZE];
};

/* DataStructure: User Query file */
struct User_query
{
    char course[BUFFSIZE];
    char category[BUFFSIZE];
};
