#ifndef __SERVER_PROTOCOL_H__
#define __SERVER_PROTOCOL_H__

#include "ftp_protocol.h"

extern int listenfd;
extern int masterfd;

/**
 * * @brief Handles the file transfer from the server to the client.
 * 
 * * @param connfd The file descriptor for the connection to the client.
 * 
 * * @return 0 to stop the while loop, 1 to continue.
 */
int file_transfer_server(int connfd);

#endif