#ifndef TFTP_CLIENT_H
#define TFTP_CLIENT_H
#include <sys/socket.h>
#include <netinet/in.h>
#include "tftp.h"

#define SUCCESS     0
#define FAILURE     1

typedef struct {
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t server_len;
    char server_ip[15];
} tftp_client_t;

// connect the sockfd to the server ip and port
void connect_to_server(tftp_client_t *client, char *ip, int port);

// handels the put operation of the client
void put_file(tftp_client_t *client);

// handels the get operation of the client
void get_file(tftp_client_t *client);

// validate the ip address
char validate_ip(char *ip);

#endif