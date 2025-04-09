/* Common file for server & client */

#include "tftp.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

extern char mode;       //from client program or server program

void send_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename) 
{
    int fd = open(filename, O_RDONLY);  //open the file to send
    if (fd == -1) {
        perror("open");
        return;
    }
    int size, netacii = 0;                         //set the size based on the mode, netacii is for netacii mode
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
    int read_size, block_number = 0;
    tftp_packet packet, ack;                    //packet and ack buffers for data transfer
    while((read_size = read(fd, packet.body.data_packet.data, size)) > 0){  //try to size bytes from the file and store in the packet
        packet.opcode = DATA;                                   //set the opcode to data
        packet.body.data_packet.block_number = block_number;    //set the block number
        if(read_size < size){                                   //if the read size is less than the packet data size
            packet.body.data_packet.data[read_size] = '\0';     //null terminate the data to indecate the end of the data
        }
        if(netacii && *packet.body.data_packet.data == '\n'){               //if the data is in netacii mode and data is '\n'                             
            packet.body.data_packet.data[0] = '\r';                         //send '\r' first
            while(1){
                //send the '\r' packet
                sendto(sockfd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, client_len);
                ack.opcode = 0;             //reset the opcode of ack
                //receive the ack
                recvfrom(sockfd, &ack, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, &client_len);
                if(ack.opcode == ACK && ack.body.ack_packet.block_number == block_number){  //if the ack is for the current block number break the loop
                    break;
                }
                else if(ack.opcode == ERROR){   //if the ack is for an error print the error message
                    printf("%s\n",ack.body.error_packet.error_msg);
                }
                else if(ack.opcode == 0){       //if timeout
                    printf("Timeout for %d block\n", block_number);
                }
            }
            block_number++;                     //increment the block number
            packet.body.data_packet.block_number++; //increment the block number
            packet.body.data_packet.data[0] = '\n';    //now send '\n'
        }
        while(1){
            //send the packet
            sendto(sockfd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, client_len);
            ack.opcode = 0;
            //receive the ack
            recvfrom(sockfd, &ack, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, &client_len);
            if(ack.opcode == ACK && ack.body.ack_packet.block_number == block_number){  //if the ack is for the current block number
                break;
            }
            else if(ack.opcode == ERROR){                                               //if the ack is for an error
                printf("%s\n",ack.body.error_packet.error_msg);
            }
            else if(ack.opcode == 0){                                                   //if timeout
                printf("Timeout for %d block\n", block_number);
            }
        }
        block_number++;                     //increment the block number
    }
    if(size==1){                            //if size is 1 send a null character a indication of end of transfer
        packet.opcode = DATA;
        packet.body.data_packet.block_number = block_number;
        packet.body.data_packet.data[0] = '\0';
        while(1){
            //send the packet
            sendto(sockfd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, client_len);
            ack.opcode = 0;
            //receive the ack
            recvfrom(sockfd, &ack, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, &client_len);
            if(ack.opcode == ACK && ack.body.ack_packet.block_number == block_number){  //if the ack is for the current block number
                break;
            }
            else if(ack.opcode == ERROR){                                               //if the ack is for an error
                printf("%s\n",ack.body.error_packet.error_msg);
            }
            else if(ack.opcode == 0){                                                   //if timeout
                printf("Timeout for %d block\n", block_number);
            }
        }
    }
    close(fd);                              //close the file
}

void receive_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename) 
{
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);    //create the file in the server
    if(fd == -1){
        perror("open");
        return;
    }
    int size = 1;                           //set the size to 1(byte) by default for mode 2 and 3
    if(mode == 1){                          //if mode is 1 set the size to 512
        size = 512;
    }
    tftp_packet packet,ack;                 //packet and ack buffers for data transfer
    socklen_t packet_size;                  //size of the packet
    int block_number = 0;
    while(1){
        //receive the packet
        recvfrom(sockfd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, &packet_size);
        if(block_number == packet.body.data_packet.block_number){       //if the block number is correct
            if(packet.body.data_packet.data[0] == '\0'){                //if the packet is the last packet break the loop
                ack.opcode = ACK;
                ack.body.ack_packet.block_number = block_number;
                //send the ack
                sendto(sockfd, &ack, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, client_len);
                break;
            }
            //write the data to file
            write(fd,packet.body.data_packet.data,strlen(packet.body.data_packet.data)<size?strlen(packet.body.data_packet.data):size);
            //set ack
            ack.opcode = ACK;
            ack.body.ack_packet.block_number = block_number;
            //send the ack
            sendto(sockfd, &ack, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, client_len);
            block_number++;                     //increment the block number
            if(strlen(packet.body.data_packet.data)<size){      //if the packet is the last packet break the loop
                close(fd);                                          //close the file and return
                return;
            }
        }
        else{                       //if not the correct block number
            ack.opcode = ERROR;
            ack.body.error_packet.error_code = block_number;
            sprintf(ack.body.error_packet.error_msg,"ERROR : block number %d not recieved\n",block_number);
            //send error
            sendto(sockfd, &ack, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, client_len);
        }
    }
    close(fd);      //close the file
}