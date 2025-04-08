#include "tftp_client.h"
#include "tftp.h"


extern char mode;

char validate_ip(char *ip){
    int temp=0,dots=0,index=0;
    while(*ip&&index<15){
        if(*ip=='.'){
            if(temp>255){
                return FAILURE;
            }
            temp=0;
            dots++;
        }
        else if(*ip>'9'||*ip<'0'){
            return FAILURE;
        }
        else{
            temp=temp*10+*ip-'0';
        }
        ip++;
        index++;
    }
    if(temp>255){
        return FAILURE;
    }
    if(dots!=3){
        return FAILURE;
    }
    return SUCCESS;
}


void put_file(tftp_client_t *client){
    tftp_packet packet,ack;
    memset(&packet, 0, sizeof(tftp_packet));
    memset(&ack, 0, sizeof(tftp_packet));
    printf("Enter the file name to upload : ");
    scanf("%255s",packet.body.request.filename);
    int fd=open(packet.body.request.filename,O_RDONLY);
    if(fd==-1){
      perror("open");
      return ;
    }
    close(fd);
    // strncpy(filename,packet.body.request.filename,255);
    packet.opcode=WRQ;
    switch(mode){
      case 1:
        strcpy(packet.body.request.mode,"NORMAL");
        break;
      case 2:
        strcpy(packet.body.request.mode,"BTYE");
        break;
      case 3:
        strcpy(packet.body.request.mode,"NETACII");
        break;
    }
    sendto(client->sockfd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)&client->server_addr, client->server_len);
    ack.opcode=0;
    recvfrom(client->sockfd, &ack, sizeof(tftp_packet), 0, (struct sockaddr *)&client->server_addr, &client->server_len);
    if(ack.opcode==ERROR){
      printf("Error: %s\n",ack.body.error_packet.error_msg);
      return;
    }
    if(ack.opcode==ACK && ack.body.ack_packet.block_number==1){
        printf("File already present in server\n");
        return;
    }
    if(ack.opcode == 0){
        printf("Time out, no response from server\ntry again\n");
        return;
    }
    send_file(client->sockfd, client->server_addr, client->server_len, packet.body.request.filename);
    printf("File uploaded successfully\n");
}

void get_file(tftp_client_t *client){
    tftp_packet packet,ack;
    memset(&packet, 0, sizeof(tftp_packet));
    memset(&ack, 0, sizeof(tftp_packet));
    printf("Enter the file name to download : ");
    scanf("%255s", packet.body.request.filename);
    int fd=open(packet.body.request.filename, O_RDONLY);
    if(fd>=0){
        close(fd);
        char op;
        printf("File with same name is already present\nOverwrite (Y/N) : ");
        scanf("%c", &op);
        if(!(op=='y'||op=='Y')){
            printf("cancelled download\n");
            return;
        }
        unlink(packet.body.request.filename);                                           //delte the existing file
    }
    packet.opcode = RRQ;
    switch(mode){
        case 1:
          strcpy(packet.body.request.mode,"NORMAL");
          break;
        case 2:
          strcpy(packet.body.request.mode,"BTYE");
          break;
        case 3:
          strcpy(packet.body.request.mode,"NETACII");
          break;
    }
    sendto(client->sockfd, &packet, sizeof(tftp_packet), 0, (struct sockaddr *)&client->server_addr, client->server_len);
    recvfrom(client->sockfd, &ack, sizeof(tftp_packet), 0, (struct sockaddr *)&client->server_addr, &client->server_len);
    if(ack.opcode ==0){
        printf("Time out, no response from server\ntry again\n");
        return;
    }
    if(ack.opcode==ERROR){
        printf("Error: %s\n",ack.body.error_packet.error_msg);
        return;
    }
    if(ack.opcode==ACK && ack.body.ack_packet.block_number==1){
        printf("File not present in server\n");
        return;
    }
    if(ack.opcode == 0){
        printf("Time out, no response from server\ntry again\n");
        return;
    }
    receive_file(client->sockfd, client->server_addr, client->server_len, packet.body.request.filename);
    printf("File downloaded successfully\n");
}


void connect_to_server(tftp_client_t *client, char *ip, int port){
    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &client->server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(client->sockfd);
        exit(EXIT_FAILURE);
    }
    client->server_len = sizeof(client->server_addr);
}
