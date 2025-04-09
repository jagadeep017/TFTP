/* Common file for server & client */

#include "tftp.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

extern char mode;

void send_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename) 
{
    printf("inside send file function\n");
    // Implement file sending logic here
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return;
    }
    int size,netacii=0;
    switch (mode){
        case 1:
            size = 512;
            break;
        case 2:
            size = 1;
            break;
        case 3:
            size = 1;
            netacii = 1;
            break;
    }
    int read_size,block_number=0;
    tftp_packet packet,ack;
    printf("starting file transfer\n");
    while((read_size = read(fd, packet.body.data_packet.data, size))>0){
        packet.opcode = DATA;
        packet.body.data_packet.block_number = block_number;
        if(read_size < size){
            packet.body.data_packet.data[read_size] = '\0';
        }
        if(netacii){
            if(*packet.body.data_packet.data == '\n'){
                packet.body.data_packet.data[0] = '\r';
                while(1){
                    printf("block %d , data %10s\n",block_number,packet.body.data_packet.data);
                    sendto(sockfd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, client_len);
                    recvfrom(sockfd, &ack, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, &client_len);
                    if(ack.opcode == ACK && ack.body.ack_packet.block_number == block_number){
                        printf("block %d sent successfully\n",block_number);
                        break;
                    }
                    else if(ack.opcode == ERROR){
                        printf("%s\n",ack.body.error_packet.error_msg);
                    }
                }
                block_number++;
                packet.body.data_packet.block_number++;
                packet.body.data_packet.data[0] = '\n';
            }
        }
        while(1){
            printf("block %d , data %10s\n",block_number,packet.body.data_packet.data);
            sendto(sockfd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, client_len);
            recvfrom(sockfd, &ack, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, &client_len);
            if(ack.opcode == ACK && ack.body.ack_packet.block_number == block_number){
                printf("block %d sent successfully\n",block_number);
                break;
            }
            else if(ack.opcode == ERROR){
                printf("%s\n",ack.body.error_packet.error_msg);
            }
        }
        printf("%d\t",read_size);
        block_number++;
    }
    if(size==1){
        tftp_packet packet,ack;
        socklen_t ack_size = sizeof(ack); 
        packet.opcode = DATA;
        packet.body.data_packet.block_number = block_number;
        packet.body.data_packet.data[0]='\0';
        while(1){
            printf("sending eof\n");
            sendto(sockfd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, client_len);
            recvfrom(sockfd, &ack, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, &ack_size);
            if(ack.opcode == ACK && ack.body.ack_packet.block_number == block_number){
                printf("eof sent\n");
                break;
            }
            else if(ack.opcode == ERROR){
                printf("%s\n",ack.body.error_packet.error_msg);
            }
        }
        block_number++;
    }
    close(fd);
}

void receive_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename) 
{
    printf("inside recieve function\n");
    // Implement file receiving logic here
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if(fd == -1){
        perror("open");
        return;
    }
    int size = 1;
    if(mode == 1){
        size = 512;
    }
    tftp_packet packet,ack;
    socklen_t packet_size;
    int block_number=0;
    while(1){
        recvfrom(sockfd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, &packet_size);
        printf("block %d , data %10s\n",packet.body.data_packet.block_number,packet.body.data_packet.data);
        if(block_number==packet.body.data_packet.block_number){
            if(packet.body.data_packet.data[0]=='\0'){
                ack.opcode = ACK;
                ack.body.ack_packet.block_number = block_number;
                printf("sending ack %d\n",block_number);
                sendto(sockfd, &ack, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, client_len);
                block_number++;
                break;
            }
            write(fd,packet.body.data_packet.data,strlen(packet.body.data_packet.data)<size?strlen(packet.body.data_packet.data):size);
            ack.opcode = ACK;
            ack.body.ack_packet.block_number = block_number;
            sendto(sockfd, &ack, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, client_len);
            block_number++;
            printf("\n%ld\n\n",strlen(packet.body.data_packet.data));
            if(strlen(packet.body.data_packet.data)<size){
                close(fd);
                return;
            }
        }
        else{
            ack.opcode = ERROR;
            ack.body.error_packet.error_code = block_number;
            sprintf(ack.body.error_packet.error_msg,"ERROR : block number %d not recieved\n",block_number);
            printf("%s\n",ack.body.error_packet.error_msg);
            sendto(sockfd, &ack, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, client_len);
        }
    }
    close(fd);
}