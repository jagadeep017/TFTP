#include "tftp_client.h"
#include "tftp.h"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

extern char mode;

char validate_ip(char *ip){
    int temp = 0, dots = 0, index = 0;
    while(*ip && index < 15){
        if(*ip == '.'){             //check for dots
            if(*(ip + 1) == '.'){   //check for consecutive dots
                return FAILURE;
            }
            if(temp > 255){         //if the accumalted value is greater than 255
                return FAILURE;
            }
            temp = 0;               //reset the accumalted value
            dots++;                 //increment the number of dots
        }
        else if(*ip > '9' || *ip < '0'){    //check if any non digit is present
            return FAILURE;
        }
        else{
            temp = temp * 10 + *ip - '0';   //accumalate the value
        }
        ip++;                       //move to the next character
        index++;
    }
    if(temp > 255){                 //at end if the accumalted value is greater than 255
        return FAILURE;
    }
    if(dots != 3){                  //validate no of dots
        return FAILURE;
    }
    return SUCCESS;                 //return success
}

void put_file(tftp_client_t *client){
    tftp_packet packet, ack;                        //packet and ack for intial request
    memset(&packet, 0, sizeof(tftp_packet));        //initialize the packet and ack
    memset(&ack, 0, sizeof(tftp_packet));
    printf("Enter the file name to upload : ");
    scanf("%255s", packet.body.request.filename);   //read the file name
    getchar();                                      //consume newline
    int fd = open(packet.body.request.filename,O_RDONLY);   //check if the file is present
    if(fd == -1){                                   //if not present return
      perror("open");
      return ;
    }
    close(fd);                                      //close the file
    packet.opcode = WRQ;                            //set the opcode
    switch(mode){                                   //set the mode
      case 1:
        strcpy(packet.body.request.mode, "NORMAL");
        break;
      case 2:
        strcpy(packet.body.request.mode, "BTYE");
        break;
      case 3:
        strcpy(packet.body.request.mode, "NETACII");
        break;
    }
    //send the request to the server
    sendto(client->sockfd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)&client->server_addr, client->server_len);

    //receive the ack from the server
    recvfrom(client->sockfd, &ack, sizeof(tftp_packet), 0, (struct sockaddr *)&client->server_addr, &client->server_len);
    if(ack.opcode == ERROR){                        //if the ack is error
      printf("Error: %s\n", ack.body.error_packet.error_msg);
      return;
    }
    if(ack.opcode == ACK && ack.body.ack_packet.block_number == 1){ //if block number is 1 in ack means file is already present in serrver
        printf("File already present in server\n");
        return;
    }
    if(ack.opcode == 0){                            //if opcode is 0 then no response in time
        printf("Time out, no response from server\ntry again\n");
        return;
    }

    //start the file transfer
    send_file(client->sockfd, client->server_addr, client->server_len, packet.body.request.filename);
    printf("File uploaded successfully\n");
}

void get_file(tftp_client_t *client){
    tftp_packet packet, ack;                        //packet and ack for intial request
    memset(&packet, 0, sizeof(tftp_packet));    //initialize the packet and ack
    memset(&ack, 0, sizeof(tftp_packet));
    printf("Enter the file name to download : ");
    scanf("%255s", packet.body.request.filename);   //read the file name
    getchar();                                      //consume newline
    int fd = open(packet.body.request.filename, O_RDONLY);   //check if the file is present
    if(fd >= 0){                                        //if present ask for overwrite
        close(fd);
        char op;
        printf("File with same name is already present\nOverwrite (Y/N) : ");
        scanf("%c", &op);
        getchar();                                          //consume newline
        if(!(op == 'y' || op == 'Y')){                      //if yes is not pressed then return
            printf("cancelled download\n");
            return;
        }
        unlink(packet.body.request.filename);                                           //delete the existing file
    }
    packet.opcode = RRQ;
    switch(mode){
        case 1:
          strcpy(packet.body.request.mode, "NORMAL");
          break;
        case 2:
          strcpy(packet.body.request.mode, "BTYE");
          break;
        case 3:
          strcpy(packet.body.request.mode, "NETACII");
          break;
    }
    //send the request to the server
    sendto(client->sockfd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)&client->server_addr, client->server_len);
    //receive the ack from the server
    recvfrom(client->sockfd, &ack, sizeof(tftp_packet), 0, (struct sockaddr *)&client->server_addr, &client->server_len);

    if(ack.opcode == 0){                                //if opcode is 0 mean no response in time
        printf("Time out, no response from server\ntry again\n");
        return;
    }
    if(ack.opcode==ERROR){                              //if the ack is error
        printf("Error: %s\n",ack.body.error_packet.error_msg);
        return;
    }
    if(ack.opcode == ACK && ack.body.ack_packet.block_number == 1){ //if block number is 1 in ack means file is not present in server
        printf("File not present in server\n");
        return;
    }
    //start the file transfer
    receive_file(client->sockfd, client->server_addr, client->server_len, packet.body.request.filename);
    printf("File downloaded successfully\n");
}

void connect_to_server(tftp_client_t *client, char *ip, int port){
    client->server_addr.sin_family = AF_INET;                   //set the family
    client->server_addr.sin_port = htons(port);                 //set the port number
    if (inet_pton(AF_INET, ip, &client->server_addr.sin_addr) <= 0) {           //convert the ip address
        perror("inet_pton failed");
        close(client->sockfd);
        exit(EXIT_FAILURE);
    }
    client->server_len = sizeof(client->server_addr);           //set the length
}
