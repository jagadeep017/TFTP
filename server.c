#include "tftp.h"
#include "tftp_server.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>


#define IP_ADDR     "127.0.0.1"
#define SERVER_PORT 6000

//collect the file name and operation from the cilent

//validtae the file if it is present(clear the data) or not(create)

//send ack to cilent

//collect the data from cilent and store in the file

//send ack to cilent

int mode=1;


int main(){
    int sock_fd;
    struct sockaddr_in server_addr, client_addr;
    // socklen_t cli_len=sizeof(tftp_packet);
    tftp_packet packet;
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    struct timeval time_out;
    time_out.tv_sec = 5;
    time_out.tv_usec = 0;
    if(setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &time_out, sizeof(time_out))){
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = ntohs(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(IP_ADDR);

    if(bind(sock_fd,(struct sockaddr *)&server_addr,sizeof(server_addr))==-1){
        perror("bind");
        exit(EXIT_FAILURE);
    }

    while(1){
        packet.opcode=0;
        struct sockaddr_in client;
        socklen_t cli_len = sizeof(struct sockaddr_in);
        recvfrom(sock_fd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)&client, &cli_len);
        
        switch(packet.opcode){
            case RRQ:
                if(!strcmp(packet.body.request.mode,"BTYE")){
                    mode = 2;
                }
                else if(!strcmp(packet.body.request.mode,"NETACII")){
                    mode = 3;
                }
                else{
                    mode =1;
                }
                send_ser(sock_fd, &packet, (struct sockaddr *)&client, &cli_len);
                break;
            case WRQ:
                if(!strcmp(packet.body.request.mode,"BTYE")){
                    mode = 2;
                }
                else if(!strcmp(packet.body.request.mode,"NETACII")){
                    mode = 3;
                }
                else{
                    mode = 1;
                }
                recv_ser(sock_fd, &packet, (struct sockaddr *)&client, &cli_len);
                break;
            // case DATA:
            //     break;
            // case ACK:
            //     break;
            // case ERROR:
            //     break;
        }
    }
    
    return 0;
}