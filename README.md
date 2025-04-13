# TFTP

## Overview
TFTP (Trivial File Transfer Protocol) is a simple protocol used for transferring files.

## Features
- Lightweight and easy to implement
- Uses UDP for data transfer
- Supports read and write operations

## compile
To compile the TFTP server and client, use the following command:
```bash
make
```
To compile debug version, use the following command:
```bash
make debug
```
to compile individual components, use the following command:
```bash
make <component>
```
where `<component>` can be `server`, `client`, `server_debug`, or `client_debug`.

## Usage
### Server
Open a terminal in the project directory.
Start the server by running:
```bash
./server/server
```
The server listens on IP address 127.0.0.1 and port 6969 for incoming TFTP requests.
### Client
Open a terminal in the project directory.
Start the client by running:
```bash
./client/client
```
The client provides a menu with the following operations:
Connect: Input the server IP address.
Put: Upload a file to the server.
Get: Download a file from the server.
Mode: Select transfer mode (NORMAL, BYTE, or NETASCII).
Exit: Quit the client.

#### Modes
- NORMAL: Standard mode for file transfer.
- BYTE: Transfers files as raw byte streams.
- NETASCII: Transfers files in ASCII format, converting line endings as necessary.
- The default mode is NORMAL.

## Files Description
- tftp.h: Contains common definitions, data structures, and function prototypes for file transfer.
- tftp.c: Implements the actual file transfer routines.
- tftp_client.h & c_fun.c: Provide implementations of client-side operations.
- tftp_server.h & s_fun.c: Provide implementations of server-side operations.
- client.c and server.c: Main entry points for the TFTP client and server respectively.

## Notes
- Ensure that the server is running before attempting to use client operations.
- The client and server executables are stored in their respective directories after compilation.
- UDP is used for file transfers, so ensure that your environment permits UDP traffic between the client and server.