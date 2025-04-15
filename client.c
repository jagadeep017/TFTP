#include "tftp.h"
#include "tftp_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char mode=1;
//nomral(1) 512 byte, Dutect(2) byte by byte, netascii(3) include \r before \n

tftp_client_t socket_c;         //to store spckfd, server_addr, server_len and server_ip


int main(){

    char op;
    memset(&socket_c, 0, sizeof(tftp_client_t));    //initialize the struct with 0

    //create socket
    if((socket_c.sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
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
        scanf("%c", &op);
        getchar();              // consume newline character
        switch(op){
            case '1':
                //read ip address from cmd
                printf("enter the ip address: ");
                scanf("%15s", socket_c.server_ip);  //read ip address
                getchar();          // consume newline character

                //validate ip address
                if(validate_ip(socket_c.server_ip) == FAILURE){
                    printf("%s is an invalid ip address\n", socket_c.server_ip);
                    continue;
                }
                connect_to_server(&socket_c, socket_c.server_ip, PORT);  //connect to server
                break;
            case '2':
                if(socket_c.server_ip[0] == '\0'){                       //check if connection is not established
                    printf("ip address unkown, please connect to a ip address\n");
                    break;
                }
                put_file(&socket_c);
                socket_c.server_addr.sin_port = htons(PORT);        //reset the port number
                break;
            case '3':
                if(socket_c.server_ip[0] == '\0'){                       //check if connection is not established
                    printf("ip address unkown, please connect to a ip address\n");
                    break;
                }
                get_file(&socket_c);
                socket_c.server_addr.sin_port = htons(PORT);        //reset the port number
                break;
            case '4':
                //print options
                printf("Enter the mode(1-3): \n1. Normal(521 bytes)\n2. Dutect(byte by byte)\n3. Netacii(byte by byte plus \\r before \\n)\n");
                mode=getchar() - '0';       //covert char to int
                getchar(); // consume newline character
                if(mode < 1 || mode > 3){           //if mode is invalid set mode to normal
                    printf("Invalid mode\nmode set to normal\n");
                    mode = 1;
                }
                break;
            case '5':                       //exit option
                printf("Exiting the program...\n");
                exit(EXIT_SUCCESS);
                break;
            default:
                break;
        }
    }
    return 0;
}