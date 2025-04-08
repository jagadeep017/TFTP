#include "tftp.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

void send_ser(int sock_fd, tftp_packet *request,  struct sockaddr_in * client, socklen_t *cli_len){
    int fd = open(request->body.request.filename,O_RDONLY);
    tftp_packet packet;
    packet.opcode = ACK;
    if(fd == -1){
        printf("File %s not avilable to send\n",request->body.request.filename);
        packet.body.ack_packet.block_number = 1;
        sendto(sock_fd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)client, *cli_len);
        return;
    }
    packet.body.ack_packet.block_number = 0;
    sendto(sock_fd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)client, *cli_len);
    send_file(sock_fd, *client, *cli_len, request->body.request.filename);
    close(fd);
}

void recv_ser(int sock_fd, tftp_packet *request, struct sockaddr_in * client, socklen_t *cli_len){
    int fd = open(request->body.request.filename,O_RDONLY);
    tftp_packet packet;
    packet.opcode = ACK;
    if(fd !=-1){
        printf("File %s already avilable in server\n",request->body.request.filename);
        packet.body.ack_packet.block_number = 1;
        sendto(sock_fd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)client, *cli_len);
        close(fd);
        return;
    }
    packet.body.ack_packet.block_number = 0;
    sendto(sock_fd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)client, *cli_len);
    receive_file(sock_fd, *client, *cli_len, request->body.request.filename);
}
