/* Common file for server & client*/

#ifndef TFTP_H
#define TFTP_H

#include <stdint.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define PORT 6969
#define BUFFER_SIZE 516  // TFTP data packet size (512 bytes data + 4 bytes header)
#define TIMEOUT_SEC 5    // Timeout in seconds
#define RETRYS      4    // No of retrys for timeouts

// TFTP OpCodes
typedef enum {
    RRQ = 1,  // Read Request
    WRQ = 2,  // Write Request
    DATA = 3, // Data Packet
    ACK = 4,  // Acknowledgment
    ERROR = 5 // Error Packet
} tftp_opcode;

// TFTP Packet Structure
typedef struct {
    uint16_t opcode; // Operation code (RRQ/WRQ/DATA/ACK/ERROR)
    union {
        struct {
            char filename[256];
            char mode[8];  // Typically "octet"
        } request;  // RRQ and WRQ
        struct {
            uint16_t block_number;
            uint16_t block_size;
            char data[512];
        } data_packet; // DATA
        struct {
            uint16_t block_number;
        } ack_packet; // ACK
        struct {
            uint16_t error_code;
            char error_msg[512];
        } error_packet; // ERROR
    } body;
} tftp_packet;

//protocal to send file using udp
void send_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename);

//protocal to receive file using udp
void receive_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename);

//function to send each packet
int send_packet(int sockfd, tftp_packet *packet, tftp_packet *ack, int block_number,struct sockaddr_in client_addr, socklen_t client_len);

#endif // TFTP_H
