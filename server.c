#include "tftp.h"
#include "tftp_server.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>


#define IP_ADDR     "127.0.0.1"               //ip address of the server

void signal_handler(int signum){                //to stop the server
    printf("\nServer stopped\n");
    exit(0);
}

int mode = 1;
//nomral(1) 512 byte, Dutect(2) byte by byte, netascii(3) include \r before \n

int main(){
    int sock_fd;                                           //socket file descriptor

    signal(SIGINT,signal_handler);            //to stop the server

    chdir("server");

    struct sockaddr_in server_addr;                     //server address

    tftp_packet packet;                                 //packet to handle requests

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);   //create socket
    if (sock_fd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    //setting timeout of 5 sec
    struct timeval time_out;
    time_out.tv_sec = 5;
    time_out.tv_usec = 0;
    if(setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &time_out, sizeof(time_out))){
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    //set up server address and port
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = ntohs(PORT);
    server_addr.sin_addr.s_addr = inet_addr(IP_ADDR);
    //bind the socket
    if(bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
        perror("bind");
        exit(EXIT_FAILURE);
    }

    while(1){
        packet.opcode = 0;                //clearing prev opcode
        struct sockaddr_in client;      //client address
        socklen_t cli_len = sizeof(struct sockaddr_in);

        //receiving request
        recvfrom(sock_fd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)&client, &cli_len);
        
        switch(packet.opcode){              //handling requests
            case RRQ:
                if(!strcmp(packet.body.request.mode, "BTYE")){   //setting mode
                    mode = 2;
                }
                else if(!strcmp(packet.body.request.mode, "NETACII")){
                    mode = 3;
                }
                else{
                    mode = 1;
                }
                //function to handle read request
                send_ser(sock_fd, &packet, (struct sockaddr *)&client, &cli_len);
                break;
            case WRQ:
                if(!strcmp(packet.body.request.mode, "BTYE")){   //setting mode
                    mode = 2;
                }
                else if(!strcmp(packet.body.request.mode, "NETACII")){
                    mode = 3;
                }
                else{
                    mode = 1;
                }
                //function to handle write request
                recv_ser(sock_fd, &packet, (struct sockaddr *)&client, &cli_len);
                break;
        }
    }
    return 0;
}