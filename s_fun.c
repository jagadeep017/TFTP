#include "tftp.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

void send_ser(int sock_fd, tftp_packet *request,  struct sockaddr_in *client, socklen_t *cli_len){
    printf("read request for file %s\n", request->body.request.filename);
    int fd = open(request->body.request.filename, O_RDONLY);    //open the file
    tftp_packet packet;         //ack packet
    packet.opcode = ACK;        //set the opcode
    if(fd == -1){               //if the file is not present
        printf("File %s not avilable to send\n", request->body.request.filename);
        packet.body.ack_packet.block_number = 1;
        //send the ack with block number as 1 suggesting that the file is not present in the server
        sendto(sock_fd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)client, *cli_len);
        return;
    }
    //else
    printf("File %s avilable to send\n", request->body.request.filename);
    packet.body.ack_packet.block_number = 0;
    //send the ack with block number as 0 suggesting that the file is present in the server
    sendto(sock_fd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)client, *cli_len);

    //start file transfer
    send_file(sock_fd, *client, *cli_len, request->body.request.filename);
    close(fd);
}

void recv_ser(int sock_fd, tftp_packet *request, struct sockaddr_in * client, socklen_t *cli_len){
    printf("write request for file %s\n", request->body.request.filename);
    int fd = open(request->body.request.filename,O_RDONLY);         //to check if the file is present
    tftp_packet packet;                                         //ack packet
    packet.opcode = ACK;                                        //set the opcode
    if(fd != -1){                                               //if the file is present
        printf("File %s already avilable in server\n",request->body.request.filename);
        packet.body.ack_packet.block_number = 1;
        //send the ack with block number as 1 suggesting that the file is present in the server
        sendto(sock_fd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)client, *cli_len);
        close(fd);                                              //close the file
        return;
    }
    //else
    packet.body.ack_packet.block_number = 0;
    //send the ack with block number as 0 suggesting that the file is not present in the server
    sendto(sock_fd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)client, *cli_len);

    //start file transfer
    receive_file(sock_fd, *client, *cli_len, request->body.request.filename);
}
