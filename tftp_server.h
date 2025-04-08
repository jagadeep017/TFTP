#ifndef TFTP_SERVER_H
#define TFTP_SERVER_H

#include "tftp.h"
#include <sys/socket.h>

void send_ser(int sock_fd, tftp_packet *request,  struct sockaddr * client, socklen_t *cli_len);

void recv_ser(int sock_fd, tftp_packet *request, struct sockaddr * client, socklen_t *cli_len);

#endif