# Mini-FTP Project

## Introduction
This project aims to develop a simplified File Transfer Protocol (FTP) system to facilitate file exchange between a client and a server. The primary objective is to familiarize students with system-level programming by involving process management, socket communication, and I/O redirection, thereby implementing a basic but functional FTP service.

## Project Structure
The project consists of multiple components that handle different aspects of the FTP system:

### Server Side
- **Master Server (`ftpmaster.c`)**: Manages slave servers, handles incoming client connections, and redirects them to available slave servers.
- **Slave Server (`ftpslave.c`)**: Handles file transfer operations directly with clients. It supports operations like sending and receiving files based on client requests.

### Client Side
- **Client (`ftpclient.c`)**: Interfaces with the user, sending file transfer requests to the server and handling responses.

### Protocol Definition
- **FTP Protocol (`ftp_protocol.h`)**: Defines the data structures and constants used in the FTP operations, including request and response formats.

### Utility Modules
- **Server Protocol (`server_protocol.c` and `server_protocol.h`)**: Implements the core file transfer logic on the server side.
- **Client Protocol (`client_protocol.c` and `client_protocol.h`)**: Implements the file transfer logic on the client side, including connecting to slave servers.

### Configuration and Management
- **Slave Management (`gestion_slaves.c` and `gestion_slaves.h`)**: Provides functionalities to manage connections between the master server and slave servers.

## Features
- **Connection Management**: Efficient handling of client-server connections using TCP sockets.
- **File Transfer**: Ability to transfer files between the client and server with basic error handling.
- **Server Redundancy**: Use of multiple slave servers to balance load and ensure availability.
- **Signal Handling**: Graceful shutdown of servers on receiving interrupt signals.

## How to Run
1. **Compile the Code**: Use the provided Makefile by running `make` command in the terminal.
2. **Start the Master Server**: Run `./ftpmaster`.
3. **Start Slave Servers**: Run `./ftpslave` with the appropriate port numbers.
4. **Run the Client**: Connect to the master server using `./ftpclient <hostname>`.

## Dependencies
- The project requires the CSAPP library for robust I/O and socket operations.

## Future Work
- Implement additional FTP commands like `PUT`, `LS`, and `RM`.
- Enhance security measures for file transfers.
- Improve the user interface for easier interaction with the FTP system.

## Conclusion
This mini-FTP project serves as a foundational step towards understanding and building more complex networked and distributed systems, highlighting key aspects of network programming and system interaction.

