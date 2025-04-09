#include "tftp.h"
#include "tftp_client.h"
#include <stdio.h>

// #define PORT_NUM        6000   //port number

char mode=1;
//nomral(1) 512 btye, Dutect(2) byte by byte, netacii(3) include \r before \n

char ip[15];                    //ip address

tftp_client_t socket_c;

int main(){

    char op;
    chdir("client");
    memset(&socket_c,0,sizeof(tftp_client_t));

    //create socket
    if((socket_c.sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0){
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    //setting timeout of 5 sec
    struct timeval time_out;
    time_out.tv_sec = 5;
    time_out.tv_usec = 0;
    if(setsockopt(socket_c.sockfd, SOL_SOCKET, SO_RCVTIMEO, &time_out, sizeof(time_out))){
        perror("setsockopt failed");
        close(socket_c.sockfd);
        exit(EXIT_FAILURE);
    }

    while(1){
        //display menu
        printf("Select an operation(1 - 5)\n1. connect\n2. put\n3. get\n4. mode\n5. exit\n");
        scanf("%c",&op);
        getchar();              // consume newline character
        switch(op){
            case '1':
                //read ip address from cmd
                printf("enter the ip address: ");
                scanf("%15s",socket_c.server_ip);   //read ip address
                getchar();          // consume newline character

                //validate ip address
                if(validate_ip(socket_c.server_ip)==FAILURE){
                    printf("%s is an invalid ip address\n",socket_c.server_ip);
                    continue;
                }
                connect_to_server(&socket_c,socket_c.server_ip,PORT);    //connect to server
                break;
            case '2':
                if(socket_c.sockfd==0){
                    printf("ip address unkown,please connect to a ip address\n");
                    break;
                }
                put_file(&socket_c);
                break;
            case '3':
                if(socket_c.sockfd==0){
                    printf("ip address unkown,please connect to a ip address\n");
                    break;
                }
                get_file(&socket_c);
                break;
            case '4':
                printf("Enter the mode(1-3): \n1. Normal\n2. Dutect\n3. Netacii\n");
                mode=getchar()-'0';
                getchar(); // consume newline character
                if(mode<1||mode>3){
                    printf("Invalid mode\nmode set to normal\n");
                    mode=1;
                }
                break;
            case '5':
                printf("Exiting the program...\n");
                exit(EXIT_SUCCESS);
                break;
            default:
                break;
        }
    }

    return 0;
}


//read file name from cmd

//validate the file(no file stop)

//send the file name to server and operation

//collect the ack from server

//read 512 byte from the file and send to server

//wait for the ack from the server

//if ack is failure send the same packet again

//if ack is success send the next packet

