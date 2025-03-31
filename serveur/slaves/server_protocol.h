#ifndef __SERVER_PROTOCOL_H__
#define __SERVER_PROTOCOL_H__

#include "ftp_protocol.h"

extern int listenfd;
extern int masterfd;

int file_transfer_server(int connfd);

#endif