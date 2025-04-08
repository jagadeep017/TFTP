#ifndef TFTP_CLIENT_H
#define TFTP_CLIENT_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "tftp.h"

#define SUCCESS     0
#define FAILURE     1

typedef struct {
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t server_len;
    char server_ip[15];
} tftp_client_t;

// Function prototypes
void connect_to_server(tftp_client_t *client, char *ip, int port);
void put_file(tftp_client_t *client);
void get_file(tftp_client_t *client);
void disconnect(tftp_client_t *client);
void process_command(tftp_client_t *client, char *command);


char validate_ip(char *ip);



void send_request(int sockfd, struct sockaddr_in server_addr, char *filename, int opcode);
void receive_request(int sockfd, struct sockaddr_in server_addr, char *filename, int opcode);

#endif
