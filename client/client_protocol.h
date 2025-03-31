#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "ftp_protocol.h"

#include "csapp.h"
#include <time.h>
#include <stdint.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#define COMMAND_LEN 15
#define SERVER_PORT 9919 // A CHANGER

/**
 * @brief Function to handle file transfer from the client to the server.
 * 
 * @param clientfd The file descriptor for the client connection.
 * @param filename The name of the file to be transferred.
 * @param type The type of request (GET, PUT, etc.). 
 *
 * This function sends a request to the server to transfer a file. It handles
 * the response from the server, including success and error codes. It also
 * measures the time taken for the transfer and prints the transfer speed.
 */
void file_transfer_client(int clientfd, char *filename, typereq_t type);


/**
 * @brief Function to handle file transfer from the server to the client.
 * 
 * @param connfd The file descriptor for the client connection.
 * @param filename The name of the file to be transferred.
 * @param type The type of request (GET, PUT, etc.).
 * 
 * This function processes the client's request for file transfer. It reads the
 * request, checks if the file exists, and sends the file to the client. It also
 * handles error cases and sends appropriate responses to the client.
 */
int analyze_command(char *buf, char *cmd, char *filename);

/**
 * @brief Function to connect to a slave server.
 * 
 * @param masterfd The file descriptor for the master server connection.
 * @return The file descriptor for the slave server connection, or -1 on error.
 */
int client_connect_to_slave(int masterfd);

#endif