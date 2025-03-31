#ifndef __GESTION_SLAVES_H__
#define __GESTION_SLAVES_H__

#include "ftp_protocol.h"

extern slave_t slaves[NB_SLAVES];

int master_connect_to_slaves(int *slaves_connected);

int redirection_to_slave(int connfd, int num_slaves_connected, int last_slave_selected);

#endif