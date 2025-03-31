#include "gestion_slaves.h"

char *slaves_config_host[NB_SLAVES] = {"Dell", "Dell", "Dell"}; // A CHANGER
char *slaves_config_port[NB_SLAVES] = {"2222", "2223", "2224"}; // A CHANGER

int master_connect_to_slaves(int *slaves_connected) {
    int i = 0;
    int slave_found = 0;
    while (i < NB_SLAVES) {
        int slave_fd = open_clientfd(slaves_config_host[i], atoi(slaves_config_port[i]));
        if (slave_fd < 0) {
            fprintf(stderr, "Error connecting to slave %s:%d\n", slaves[i].hostname, slaves[i].port);
        } else {
            slaves[slave_found].fd = slave_fd;
            slaves[slave_found].hostname = strdup(slaves_config_host[i]);
            slaves[slave_found].port = atoi(slaves_config_port[i]);
            slaves[slave_found].available = AVAILBLE;
            fprintf(stdout, "Connected to slave %s:%d\n", slaves[slave_found].hostname, slaves[slave_found].port);
            slave_found++;
        }
        i++;
    }   
    *slaves_connected = slave_found;
    return slave_found == 0  ? 0 : 1;
}

int redirection_to_slave(int connfd, int num_slave_connected, int last_slave_selected) {
    slave_info_t info;
    for (int slave_index = 0; slave_index < num_slave_connected; slave_index++) {
        
        int i = (last_slave_selected + slave_index + 1) % num_slave_connected;
        if (slaves[slave_index].available == AVAILBLE) {
            info.slave_available = htonl(1);
            strcpy(info.hostname, slaves[i].hostname);
            info.port = htonl(slaves[i].port);
            Rio_writen(connfd, &info, sizeof(slave_info_t));
            return i;
        } else {
            fprintf(stderr, "Eslave %s:%d is busy\n", slaves[i].hostname, slaves[i].port);
        }
    }

    // No slave available
    info.slave_available = 0;
    info.hostname[0] = '\0';
    info.port = 0;
    Rio_writen(connfd, &info, sizeof(slave_info_t));
    return 0;
}