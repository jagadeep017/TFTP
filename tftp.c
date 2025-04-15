/* Common file for server & client */

#include "tftp.h"
#include "tftp_client.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

extern char mode;       //from client program or server program

int send_packet(int sockfd, tftp_packet *packet, tftp_packet *ack, int block_number,struct sockaddr_in client_addr, socklen_t client_len){
    int timeouts = RETRYS;           //no of retrys before stoping transmission
    while(1){
        if(!timeouts){
            printf("Transmission stopped because of too many timeouts\n");
            return FAILURE;
        }
        //send the '\r' packet
        sendto(sockfd, packet, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, client_len);
        ack->opcode = 0;             //reset the opcode of ack
        //receive the ack
        recvfrom(sockfd, ack, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, &client_len);
        if(ack->opcode == ACK && ack->body.ack_packet.block_number == block_number){  //if the ack is for the current block number break the loop
            timeouts = RETRYS;
            return SUCCESS;
        }
        else if(ack->opcode == ERROR){   //if the ack is for an error print the error message
            printf("%s\n",ack->body.error_packet.error_msg);
        }
        else if(ack->opcode == 0){       //if timeout
            printf("Timeout for %d block\n", block_number);
        }
        timeouts--;
    }
    return SUCCESS;
}

void send_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename) 
{
    int fd = open(filename, O_RDONLY);  //open the file to send
    int timeouts = RETRYS;           //no of retrys before stoping transmission
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
        packet.body.data_packet.block_size = read_size;
        if(netacii && *packet.body.data_packet.data == '\n'){               //if the data is in netacii mode and data is '\n'                             
            packet.body.data_packet.data[0] = '\r';                         //send '\r' first
            //sending the packet
            if(send_packet(sockfd, &packet, &ack, block_number, client_addr, client_len) == FAILURE){
                //if failed to send packet stop the transmission
                close(fd);
                return;
            }
            block_number++;                     //increment the block number
            packet.body.data_packet.block_number++; //increment the block number
            packet.body.data_packet.data[0] = '\n';    //now send '\n'
        }
        //sending the packet
        if(send_packet(sockfd, &packet, &ack, block_number, client_addr, client_len) == FAILURE){
            //if failed to send packet stop the transmission
            close(fd);
            return;
        }
        block_number++;                     //increment the block number
        // return;
    }
    if(size==1){                            //if size is 1 send a empty data packet to indicate end of file
        packet.opcode = DATA;
        packet.body.data_packet.block_number = block_number;
        packet.body.data_packet.block_size = 0;
        //sending the packet
        if(send_packet(sockfd, &packet, &ack, block_number, client_addr, client_len) == FAILURE){
            //if failed to send packet stop the transmission
            close(fd);
            return;
        }
    }
    close(fd);                              //close the file
    printf("File %s sent successfully\n\n", filename);
}

void receive_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename) 
{
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);    //create the file in the server
    int timeouts = RETRYS;           //no of retrys before stoping transmission
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
    int block_number = 0, data_size;
    while(1){
        if(!timeouts){                      //when timeouts are over
            close(fd);
            printf("Transmission stopped due to excuss timeout\n");
            return;
        }
        //receive the packet
        recvfrom(sockfd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, &packet_size);
        if(packet.opcode == DATA && block_number == packet.body.data_packet.block_number){       //if the block number is correct
            if(packet.body.data_packet.block_size == 0){                //if the packet is the last packet break the loop
                ack.opcode = ACK;
                ack.body.ack_packet.block_number = block_number;
                //send the ack
                sendto(sockfd, &ack, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, client_len);
                break;
            }
            //write the data to file
            write(fd, packet.body.data_packet.data, packet.body.data_packet.block_size);
            //set ack
            ack.opcode = ACK;
            ack.body.ack_packet.block_number = block_number;
            //send the ack
            sendto(sockfd, &ack, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, client_len);
            block_number++;                     //increment the block number
            if(packet.body.data_packet.block_size < size){      //if the packet is the last packet break the loop
                break;
            }
            timeouts = RETRYS;
        }
        else{                       //if not the correct block number
            ack.opcode = ERROR;
            ack.body.error_packet.error_code = block_number;
            sprintf(ack.body.error_packet.error_msg,"ERROR : block number %d not recieved\n", block_number);
            //send error
            sendto(sockfd, &ack, sizeof(tftp_packet), 0, (struct sockaddr *)&client_addr, client_len);
            timeouts--;
        }
    }
    close(fd);      //close the file
    printf("File %s received successfully\n\n", filename);
}