#include "server_protocol.h"

void sigint_handler(int sig) {
    slave_status_t status = DEAD;
    fprintf(stderr, "Shutting down...\n");
    // Envoi du message de déconnexion au master
    if (masterfd == -1) {
        exit(0);
    }
    Rio_writen(masterfd, &status, sizeof(slave_status_t));
    close(listenfd);
    close(masterfd);
    exit(0);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Use: %s <slave_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    slave_status_t status;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    clientlen = (socklen_t)sizeof(clientaddr);
    uint32_t port = atoi(argv[1]);

    // Pour la connexion au master
    listenfd = Open_listenfd(port);
    if (listenfd < 0) {
        fprintf(stderr, "Error opening socket\n");
        exit(EXIT_FAILURE);
    }
    Signal(SIGINT, sigint_handler);


    fd_set fdset;
    // On attend la connexion du master
    masterfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    
    FD_ZERO(&fdset);
    FD_SET(listenfd, &fdset);
    FD_SET(masterfd, &fdset);
    int maxfd = (listenfd > masterfd) ? listenfd : masterfd;

    int connfd;
    while (1) {
        // Pour ce moment le slave est toujours disponible
        status = AVAILBLE;
        Rio_writen(masterfd, &status, sizeof(slave_status_t));

        fd_set read_fds = fdset; // Copie de fdset pour select

        // Select sur les descripteurs de fichiers
        select(maxfd + 1, &read_fds, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &read_fds)) {
            // Un client se connecte
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

            status = BUSY;
            Rio_writen(masterfd, &status, sizeof(slave_status_t));
            
            while (file_transfer_server(connfd));
            Close(connfd);
        }
        
        // Un message du master
        if (FD_ISSET(masterfd, &read_fds)) {
            slave_status_t recv_status;
            Rio_readn(masterfd, &recv_status, sizeof(slave_status_t));
            if (recv_status == DEAD) {
                fprintf(stderr, "Master disconnected\n");
                break;
            }
            Close(listenfd);
            Close(connfd);
            exit(0);
        }
    }
}