#ifndef __GESTION_SLAVES_H__
#define __GESTION_SLAVES_H__

#include "ftp_protocol.h"

extern slave_t slaves[NB_SLAVES];

/**
 * @brief Connects to the slaves and returns the number of slaves connected.
 * 
 * @param slaves_connected Pointer to store the number of slaves connected.
 * @return int 1 if at least one slave is connected, 0 otherwise.
 */
int master_connect_to_slaves(int *slaves_connected);

/**
 * @brief Redirects the connection to a slave.
 * 
 * @param connfd The file descriptor for the connection to the client.
 * @param num_slaves_connected The number of slaves connected.
 * @param last_slave_selected The index of the last selected slave.
 * @return int The index of the selected slave.
 */
int redirection_to_slave(int connfd, int num_slaves_connected, int last_slave_selected);

#endif