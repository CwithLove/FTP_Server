#include "gestion_slaves.h"

#define MAX_NAME_LEN 256

int listenfd;
int num_slaves_connected;

slave_t slaves[NB_SLAVES];

void sigint_handler(int sig) {
    slave_status_t status = DEAD;
    close(listenfd);
    for (int i = 0; i < num_slaves_connected; i++) {
        if (slaves[i].hostname != NULL) {
            write(slaves[i].fd, &status, sizeof(slave_status_t));
        }
    }
    exit(0);
}

int main(int argc, char** argv) {
    uint8_t status;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    int port;
    int last_slave_selected = 0;
    
    clientlen = (socklen_t)sizeof(clientaddr);
    port = PORT;

    // Etablissement de la connexion avec les esclaves
    if (!master_connect_to_slaves(&num_slaves_connected)) {
        fprintf(stderr, "Error connecting to slaves\n");
        exit(EXIT_FAILURE);
    }

    Signal(SIGINT, sigint_handler);
    listenfd = Open_listenfd(port);

    fd_set fdset;
    int last_fd = -1;
    FD_ZERO(&fdset);
    FD_SET(listenfd, &fdset);
    last_fd = listenfd;

    for (int i = 0; i < num_slaves_connected; i++) {
        FD_SET(slaves[i].fd, &fdset);
        if (slaves[i].fd > last_fd) {
            last_fd = slaves[i].fd;
        }
    }

    while(1) {
        fd_set read_fds = fdset; // Copie de fdset pour select
        // On attend une connexion sur le socket d'écoute ou sur un des esclaves
        // On utilise select pour gérer les connexions multiples
        int nready = select(last_fd + 1, &read_fds, NULL, NULL, NULL);
        if (nready < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        // Un client se connecte
        if (FD_ISSET(listenfd, &read_fds)) {
            int connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            last_slave_selected = redirection_to_slave(connfd, num_slaves_connected, last_slave_selected);
            continue;
        }

        // Mise à jour de l'état des esclaves
        // On vérifie si un des esclaves a envoyé une mise à jour de son état
        for (int i = 0; i < num_slaves_connected; i++) {
            if (FD_ISSET(slaves[i].fd, &read_fds)) {
                Rio_readn(slaves[i].fd, &status, sizeof(slave_status_t));
                slaves[i].available = status; 
            }

        }
    }

}